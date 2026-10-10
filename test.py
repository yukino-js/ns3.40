#! /usr/bin/env python3
import argparse
import fnmatch
import os
import shutil
import signal
import subprocess
import sys
import threading
import time
import xml.etree.ElementTree as ET


from utils import get_list_from_file

args = None

colors_lst = {
    "USE": True,
    "BOLD": "\x1b[01;1m",
    "RED": "\x1b[01;31m",
    "GREEN": "\x1b[32m",
    "YELLOW": "\x1b[33m",
    "PINK": "\x1b[35m",
    "BLUE": "\x1b[01;34m",
    "CYAN": "\x1b[36m",
    "GREY": "\x1b[37m",
    "NORMAL": "\x1b[0m",
    "cursor_on": "\x1b[?25h",
    "cursor_off": "\x1b[?25l",
}


def get_color(cl):
    if colors_lst["USE"]:
        return colors_lst.get(cl, "")
    return ""


class color_dict(object):
    def __getattr__(self, a):
        return get_color(a)

    def __call__(self, a):
        return get_color(a)


colors = color_dict()

try:
    import queue
except ImportError:
    import Queue as queue
interesting_config_items = [
    "NS3_ENABLED_MODULES",
    "NS3_ENABLED_CONTRIBUTED_MODULES",
    "NS3_MODULE_PATH",
    "ENABLE_REAL_TIME",
    "ENABLE_EXAMPLES",
    "ENABLE_TESTS",
    "EXAMPLE_DIRECTORIES",
    "ENABLE_PYTHON_BINDINGS",
    "NSCLICK",
    "ENABLE_BRITE",
    "ENABLE_OPENFLOW",
    "APPNAME",
    "BUILD_PROFILE",
    "VERSION",
    "PYTHON",
    "VALGRIND_FOUND",
]

ENABLE_REAL_TIME = False
ENABLE_EXAMPLES = True
ENABLE_TESTS = True
NSCLICK = False
ENABLE_BRITE = False
ENABLE_OPENFLOW = False
ENABLE_PYTHON_BINDINGS = False
EXAMPLE_DIRECTORIES = []
APPNAME = ""
BUILD_PROFILE = ""
BUILD_PROFILE_SUFFIX = ""
VERSION = ""
PYTHON = ""
VALGRIND_FOUND = True

test_runner_name = "test-runner"

core_kinds = ["core", "performance", "system", "unit"]

core_valgrind_skip_tests = [
    "routing-click",
    "lte-rr-ff-mac-scheduler",
    "lte-tdmt-ff-mac-scheduler",
    "lte-fdmt-ff-mac-scheduler",
    "lte-pf-ff-mac-scheduler",
    "lte-tta-ff-mac-scheduler",
    "lte-fdbet-ff-mac-scheduler",
    "lte-ttbet-ff-mac-scheduler",
    "lte-fdtbfq-ff-mac-scheduler",
    "lte-tdtbfq-ff-mac-scheduler",
    "lte-pss-ff-mac-scheduler",
]


def parse_examples_to_run_file(
    examples_to_run_path,
    cpp_executable_dir,
    python_script_dir,
    example_tests,
    example_names_original,
    python_tests,
):
    if os.path.exists(examples_to_run_path):
        cpp_examples = get_list_from_file(examples_to_run_path, "cpp_examples")
        for example_name, do_run, do_valgrind_run in cpp_examples:
            example_name_original = example_name
            example_name_parts = example_name.split(" ", 1)
            if len(example_name_parts) == 1:
                example_name = example_name_parts[0]
                example_arguments = ""
            else:
                example_name = example_name_parts[0]
                example_arguments = example_name_parts[1]

            example_path = "%s%s-%s%s" % (
                APPNAME,
                VERSION,
                example_name,
                BUILD_PROFILE_SUFFIX,
            )

            example_path = os.path.join(cpp_executable_dir, example_path)
            example_path += ".exe" if sys.platform == "win32" else ""
            example_name = os.path.join(
                os.path.relpath(cpp_executable_dir, NS3_BUILDDIR), example_name
            )
            if os.path.exists(example_path):
                if len(example_name_parts) != 1:
                    example_path = "%s %s" % (example_path, example_arguments)
                    example_name = "%s %s" % (example_name, example_arguments)

                example_tests.append(
                    (example_name, example_path, do_run, do_valgrind_run)
                )
                example_names_original.append(example_name_original)

        python_examples = get_list_from_file(examples_to_run_path, "python_examples")
        for example_name, do_run in python_examples:
            example_name_parts = example_name.split(" ", 1)
            if len(example_name_parts) == 1:
                example_name = example_name_parts[0]
                example_arguments = ""
            else:
                example_name = example_name_parts[0]
                example_arguments = example_name_parts[1]

            example_path = os.path.join(python_script_dir, example_name)

            if os.path.exists(example_path):
                if len(example_name_parts) != 1:
                    example_path = "%s %s" % (example_path, example_arguments)

                python_tests.append((example_path, do_run))


TMP_OUTPUT_DIR = "testpy-output"


def read_test(test):
    result = test.find("Result").text
    name = test.find("Name").text
    if not test.find("Reason") is None:
        reason = test.find("Reason").text
    else:
        reason = ""
    if not test.find("Time") is None:
        time_real = test.find("Time").get("real")
    else:
        time_real = ""
    return (result, name, reason, time_real)


def node_to_text(test, f, test_type="Suite"):
    (result, name, reason, time_real) = read_test(test)
    if reason:
        reason = " (%s)" % reason

    output = '%s: Test %s "%s" (%s)%s\n' % (result, test_type, name, time_real, reason)
    f.write(output)
    for details in test.findall("FailureDetails"):
        f.write("    Details:\n")
        f.write("      Message:   %s\n" % details.find("Message").text)
        f.write("      Condition: %s\n" % details.find("Condition").text)
        f.write("      Actual:    %s\n" % details.find("Actual").text)
        f.write("      Limit:     %s\n" % details.find("Limit").text)
        f.write("      File:      %s\n" % details.find("File").text)
        f.write("      Line:      %s\n" % details.find("Line").text)
    for child in test.findall("Test"):
        node_to_text(child, f, "Case")


def translate_to_text(results_file, text_file):
    text_file += ".txt" if ".txt" not in text_file else ""
    print('Writing results to text file "%s"...' % text_file, end="")
    et = ET.parse(results_file)

    with open(text_file, "w", encoding="utf-8") as f:
        for test in et.findall("Test"):
            node_to_text(test, f)

        for example in et.findall("Example"):
            result = example.find("Result").text
            name = example.find("Name").text
            if not example.find("Time") is None:
                time_real = example.find("Time").get("real")
            else:
                time_real = ""
            output = '%s: Example "%s" (%s)\n' % (result, name, time_real)
            f.write(output)

    print("done.")


def translate_to_html(results_file, html_file):
    html_file += ".html" if ".html" not in html_file else ""
    print("Writing results to html file %s..." % html_file, end="")

    with open(html_file, "w", encoding="utf-8") as f:
        f.write("<html>\n")
        f.write("<body>\n")
        f.write("<center><h1>ns-3 Test Results</h1></center>\n")

        et = ET.parse(results_file)

        f.write("<h2>Test Suites</h2>\n")
        for suite in et.findall("Test"):
            (result, name, reason, time) = read_test(suite)

            if result == "PASS":
                f.write(
                    '<h3 style="color:green">%s: %s (%s)</h3>\n' % (result, name, time)
                )
            elif result == "SKIP":
                f.write(
                    '<h3 style="color:#ff6600">%s: %s (%s) (%s)</h3>\n'
                    % (result, name, time, reason)
                )
            else:
                f.write(
                    '<h3 style="color:red">%s: %s (%s)</h3>\n' % (result, name, time)
                )

            f.write('<table border="1">\n')

            f.write("<th> Result </th>\n")

            if result in ["CRASH", "SKIP", "VALGR"]:
                f.write("<tr>\n")
                if result == "SKIP":
                    f.write('<td style="color:#ff6600">%s</td>\n' % result)
                else:
                    f.write('<td style="color:red">%s</td>\n' % result)
                f.write("</tr>\n")
                f.write("</table>\n")
                continue

            f.write("<th>Test Case Name</th>\n")
            f.write("<th> Time </th>\n")

            if result == "FAIL":
                f.write("<th>Failure Details</th>\n")

            for case in suite.findall("Test"):
                (result, name, reason, time) = read_test(case)

                if result == "FAIL":
                    first_row = True
                    for details in case.findall("FailureDetails"):
                        f.write("<tr>\n")

                        if first_row:
                            first_row = False
                            f.write('<td style="color:red">%s</td>\n' % result)
                            f.write("<td>%s</td>\n" % name)
                            f.write("<td>%s</td>\n" % time)
                        else:
                            f.write("<td></td>\n")
                            f.write("<td></td>\n")
                            f.write("<td></td>\n")

                        f.write("<td>")
                        f.write("<b>Message: </b>%s, " % details.find("Message").text)
                        f.write(
                            "<b>Condition: </b>%s, " % details.find("Condition").text
                        )
                        f.write("<b>Actual: </b>%s, " % details.find("Actual").text)
                        f.write("<b>Limit: </b>%s, " % details.find("Limit").text)
                        f.write("<b>File: </b>%s, " % details.find("File").text)
                        f.write("<b>Line: </b>%s" % details.find("Line").text)
                        f.write("</td>\n")

                        f.write("</td>\n")
                else:
                    f.write("<tr>\n")
                    f.write('<td style="color:green">%s</td>\n' % result)
                    f.write("<td>%s</td>\n" % name)
                    f.write("<td>%s</td>\n" % time)
                    f.write("<td>%s</td>\n" % reason)
                    f.write("</tr>\n")
            f.write("</table>\n")

        f.write("<h2>Examples</h2>\n")

        f.write('<table border="1">\n')

        f.write("<th> Result </th>\n")
        f.write("<th>Example Name</th>\n")
        f.write("<th>Elapsed Time</th>\n")
        f.write("<th>Details</th>\n")

        for example in et.findall("Example"):
            f.write("<tr>\n")

            (result, name, reason, time) = read_test(example)

            if result == "PASS":
                f.write('<td style="color:green">%s</td>\n' % result)
            elif result == "SKIP":
                f.write('<td style="color:#ff6600">%s</fd>\n' % result)
            else:
                f.write('<td style="color:red">%s</td>\n' % result)

            f.write("<td>%s</td>\n" % name)

            f.write("<td>%s</td>\n" % time)

            f.write("<td>%s</td>\n" % reason)

            f.write("</tr>\n")

        f.write("</table>\n")

        f.write("</body>\n")
        f.write("</html>\n")

    print("done.")


thread_exit = False


def sigint_hook(signal, frame):
    global thread_exit
    thread_exit = True
    return 0


def read_ns3_config():
    lock_filename = ".lock-ns3_%s_build" % sys.platform

    try:
        with open(lock_filename, "rt", encoding="utf-8") as f:
            for line in f:
                if line.startswith("top_dir ="):
                    key, val = line.split("=")
                    top_dir = eval(val.strip())
                if line.startswith("out_dir ="):
                    key, val = line.split("=")
                    out_dir = eval(val.strip())

    except FileNotFoundError:
        print(
            "The .lock-ns3 file was not found.  You must configure before running test.py.",
            file=sys.stderr,
        )
        sys.exit(2)

    global NS3_BASEDIR
    NS3_BASEDIR = top_dir
    global NS3_BUILDDIR
    NS3_BUILDDIR = out_dir

    with open(lock_filename, encoding="utf-8") as f:
        for line in f.readlines():
            for item in interesting_config_items:
                if line.startswith(item):
                    exec(line, globals())

    if args.verbose:
        for item in interesting_config_items:
            print("%s ==" % item, eval(item))


def make_paths():
    have_DYLD_LIBRARY_PATH = False
    have_LD_LIBRARY_PATH = False
    have_PATH = False
    have_PYTHONPATH = False

    keys = list(os.environ.keys())
    for key in keys:
        if key == "DYLD_LIBRARY_PATH":
            have_DYLD_LIBRARY_PATH = True
        if key == "LD_LIBRARY_PATH":
            have_LD_LIBRARY_PATH = True
        if key == "PATH":
            have_PATH = True
        if key == "PYTHONPATH":
            have_PYTHONPATH = True

    pypath = os.environ["PYTHONPATH"] = os.path.join(NS3_BUILDDIR, "bindings", "python")

    if not have_PYTHONPATH:
        os.environ["PYTHONPATH"] = pypath
    else:
        os.environ["PYTHONPATH"] += ":" + pypath

    if args.verbose:
        print('os.environ["PYTHONPATH"] == %s' % os.environ["PYTHONPATH"])

    if sys.platform == "darwin":
        if not have_DYLD_LIBRARY_PATH:
            os.environ["DYLD_LIBRARY_PATH"] = ""
        for path in NS3_MODULE_PATH:
            os.environ["DYLD_LIBRARY_PATH"] += ":" + path
        if args.verbose:
            print(
                'os.environ["DYLD_LIBRARY_PATH"] == %s'
                % os.environ["DYLD_LIBRARY_PATH"]
            )
    elif sys.platform == "win32":
        if not have_PATH:
            os.environ["PATH"] = ""
        for path in NS3_MODULE_PATH:
            os.environ["PATH"] += ";" + path
        if args.verbose:
            print('os.environ["PATH"] == %s' % os.environ["PATH"])
    elif sys.platform == "cygwin":
        if not have_PATH:
            os.environ["PATH"] = ""
        for path in NS3_MODULE_PATH:
            os.environ["PATH"] += ":" + path
        if args.verbose:
            print('os.environ["PATH"] == %s' % os.environ["PATH"])
    else:
        if not have_LD_LIBRARY_PATH:
            os.environ["LD_LIBRARY_PATH"] = ""
        for path in NS3_MODULE_PATH:
            os.environ["LD_LIBRARY_PATH"] += ":" + str(path)
        if args.verbose:
            print('os.environ["LD_LIBRARY_PATH"] == %s' % os.environ["LD_LIBRARY_PATH"])


VALGRIND_SUPPRESSIONS_FILE = None


def run_job_synchronously(shell_command, directory, valgrind, is_python, build_path=""):
    if VALGRIND_SUPPRESSIONS_FILE is not None:
        suppressions_path = os.path.join(NS3_BASEDIR, VALGRIND_SUPPRESSIONS_FILE)

    if is_python:
        path_cmd = PYTHON[0] + " " + os.path.join(NS3_BASEDIR, shell_command)
    else:
        if len(build_path):
            path_cmd = os.path.join(build_path, shell_command)
        else:
            path_cmd = os.path.join(NS3_BUILDDIR, shell_command)

    if valgrind:
        if VALGRIND_SUPPRESSIONS_FILE:
            cmd = (
                "valgrind --suppressions=%s --leak-check=full --show-reachable=yes --error-exitcode=2 --errors-for-leak-kinds=all %s"
                % (suppressions_path, path_cmd)
            )
        else:
            cmd = (
                "valgrind --leak-check=full --show-reachable=yes --error-exitcode=2 --errors-for-leak-kinds=all %s"
                % (path_cmd)
            )
    else:
        cmd = path_cmd

    if args.verbose:
        print("Synchronously execute %s" % cmd)

    start_time = time.time()
    proc = subprocess.Popen(
        cmd, shell=True, cwd=directory, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    stdout_results, stderr_results = proc.communicate()
    elapsed_time = time.time() - start_time

    retval = proc.returncode

    def decode_stream_results(stream_results: bytes, stream_name: str) -> str:
        try:
            stream_results = stream_results.decode()
        except UnicodeDecodeError:

            def decode(byte_array: bytes):
                try:
                    byte_array.decode()
                except UnicodeDecodeError:
                    return byte_array

            non_utf8_lines = list(
                map(lambda line: decode(line), stream_results.splitlines())
            )
            non_utf8_lines = list(filter(lambda line: line is not None, non_utf8_lines))
            print(
                f"Non-decodable characters found in {stream_name} output of {cmd}: {non_utf8_lines}"
            )

            stream_results = stream_results.decode(errors="backslashreplace")
        return stream_results

    stdout_results = decode_stream_results(stdout_results, "stdout")
    stderr_results = decode_stream_results(stderr_results, "stderr")

    if args.verbose:
        print("Return code = ", retval)
        print("stderr = ", stderr_results)

    return (retval, stdout_results, stderr_results, elapsed_time)


class Job:
    def __init__(self):
        self.is_break = False
        self.is_skip = False
        self.skip_reason = ""
        self.is_example = False
        self.is_pyexample = False
        self.shell_command = ""
        self.display_name = ""
        self.basedir = ""
        self.tempdir = ""
        self.cwd = ""
        self.tmp_file_name = ""
        self.returncode = False
        self.elapsed_time = 0
        self.build_path = ""

    def set_is_break(self, is_break):
        self.is_break = is_break

    def set_is_skip(self, is_skip):
        self.is_skip = is_skip

    def set_skip_reason(self, skip_reason):
        self.skip_reason = skip_reason

    def set_is_example(self, is_example):
        self.is_example = is_example

    def set_is_pyexample(self, is_pyexample):
        self.is_pyexample = is_pyexample

    def set_shell_command(self, shell_command):
        self.shell_command = shell_command

    def set_build_path(self, build_path):
        self.build_path = build_path

    def set_display_name(self, display_name):
        self.display_name = display_name

    def set_basedir(self, basedir):
        self.basedir = basedir

    def set_tempdir(self, tempdir):
        self.tempdir = tempdir

    def set_cwd(self, cwd):
        self.cwd = cwd

    def set_tmp_file_name(self, tmp_file_name):
        self.tmp_file_name = tmp_file_name

    def set_returncode(self, returncode):
        self.returncode = returncode

    def set_elapsed_time(self, elapsed_time):
        self.elapsed_time = elapsed_time


class worker_thread(threading.Thread):
    def __init__(self, input_queue, output_queue):
        threading.Thread.__init__(self)
        self.input_queue = input_queue
        self.output_queue = output_queue

    def run(self):
        while True:
            job = self.input_queue.get()
            if job.is_break:
                return
            if thread_exit == True:
                job.set_is_break(True)
                self.output_queue.put(job)
                continue

            if job.is_skip:
                if args.verbose:
                    print("Skip %s" % job.shell_command)
                self.output_queue.put(job)
                continue

            else:
                if args.verbose:
                    print("Launch %s" % job.shell_command)

                if job.is_example or job.is_pyexample:
                    (job.returncode, job.standard_out, job.standard_err, et) = (
                        run_job_synchronously(
                            job.shell_command,
                            job.cwd,
                            args.valgrind,
                            job.is_pyexample,
                            job.build_path,
                        )
                    )
                else:
                    if args.update_data:
                        update_data = "--update-data"
                    else:
                        update_data = ""
                    (job.returncode, job.standard_out, job.standard_err, et) = (
                        run_job_synchronously(
                            job.shell_command
                            + " --xml --tempdir=%s --out=%s %s"
                            % (job.tempdir, job.tmp_file_name, update_data),
                            job.cwd,
                            args.valgrind,
                            False,
                        )
                    )

                job.set_elapsed_time(et)

                if args.verbose:
                    print("returncode = %d" % job.returncode)
                    print("---------- begin standard out ----------")
                    print(job.standard_out)
                    print("---------- begin standard err ----------")
                    print(job.standard_err)
                    print("---------- end standard err ----------")

                self.output_queue.put(job)


def load_previously_successful_tests():
    import glob

    previously_run_tests_to_skip = {"test": [], "example": []}
    previous_results = glob.glob(f"{TMP_OUTPUT_DIR}/*-results.xml")
    if not previous_results:
        print("No previous runs to rerun")
        exit(-1)
    latest_result_file = list(
        sorted(previous_results, key=lambda x: os.path.basename(x), reverse=True)
    )[0]

    try:
        previous_run_results = ET.parse(latest_result_file)
    except ET.ParseError:
        print(f"Failed to parse XML {latest_result_file}")
        exit(-1)

    for test_type in ["Test", "Example"]:
        if previous_run_results.find(test_type):
            temp = list(
                map(
                    lambda x: (x.find("Name").text, x.find("Result").text),
                    previous_run_results.findall(test_type),
                )
            )
            temp = list(filter(lambda x: x[1] in ["PASS", "SKIP"], temp))
            temp = [x[0] for x in temp]
            previously_run_tests_to_skip[test_type.lower()] = temp
    return previously_run_tests_to_skip


def run_tests():
    read_ns3_config()

    global BUILD_PROFILE_SUFFIX
    if BUILD_PROFILE == "release":
        BUILD_PROFILE_SUFFIX = ""
    else:
        BUILD_PROFILE_SUFFIX = "-" + BUILD_PROFILE

    test_runner_name = "%s%s-%s%s" % (
        APPNAME,
        VERSION,
        "test-runner",
        BUILD_PROFILE_SUFFIX,
    )
    test_runner_name += ".exe" if sys.platform == "win32" else ""

    if not args.no_build:
        if len(args.example):
            build_cmd = "./ns3 build %s" % os.path.basename(args.example)
        else:
            build_cmd = "./ns3"

        if sys.platform == "win32":
            build_cmd = sys.executable + " " + build_cmd

        if args.verbose:
            print("Building: %s" % build_cmd)

        proc = subprocess.run(build_cmd, shell=True)
        if proc.returncode:
            print("ns3 died. Not running tests", file=sys.stderr)
            return proc.returncode

    make_paths()

    lock_filename = ".lock-ns3_%s_build" % sys.platform
    if os.path.exists(lock_filename):
        ns3_runnable_programs = get_list_from_file(
            lock_filename, "ns3_runnable_programs"
        )
        ns3_runnable_scripts = get_list_from_file(lock_filename, "ns3_runnable_scripts")
        ns3_runnable_scripts = [
            os.path.basename(script) for script in ns3_runnable_scripts
        ]
    else:
        print(
            "The build status file was not found.  You must configure before running test.py.",
            file=sys.stderr,
        )
        sys.exit(2)

    ns3_runnable_programs_dictionary = {}
    for program in ns3_runnable_programs:
        program_name = os.path.basename(program)
        ns3_runnable_programs_dictionary[program_name] = program

    example_tests = []
    example_names_original = []
    python_tests = []
    for directory in EXAMPLE_DIRECTORIES:
        example_directory = os.path.join("examples", directory)
        examples_to_run_path = os.path.join(example_directory, "examples-to-run.py")
        cpp_executable_dir = os.path.join(NS3_BUILDDIR, example_directory)
        python_script_dir = os.path.join(example_directory)

        parse_examples_to_run_file(
            examples_to_run_path,
            cpp_executable_dir,
            python_script_dir,
            example_tests,
            example_names_original,
            python_tests,
        )

    for module in NS3_ENABLED_MODULES:
        module = module[len("ns3-") :]

        module_directory = os.path.join("src", module)
        example_directory = os.path.join(module_directory, "examples")
        examples_to_run_path = os.path.join(
            module_directory, "test", "examples-to-run.py"
        )
        cpp_executable_dir = os.path.join(NS3_BUILDDIR, example_directory)
        python_script_dir = os.path.join(example_directory)

        parse_examples_to_run_file(
            examples_to_run_path,
            cpp_executable_dir,
            python_script_dir,
            example_tests,
            example_names_original,
            python_tests,
        )

    for module in NS3_ENABLED_CONTRIBUTED_MODULES:
        module = module[len("ns3-") :]

        module_directory = os.path.join("contrib", module)
        example_directory = os.path.join(module_directory, "examples")
        examples_to_run_path = os.path.join(
            module_directory, "test", "examples-to-run.py"
        )
        cpp_executable_dir = os.path.join(NS3_BUILDDIR, example_directory)
        python_script_dir = os.path.join(example_directory)

        parse_examples_to_run_file(
            examples_to_run_path,
            cpp_executable_dir,
            python_script_dir,
            example_tests,
            example_names_original,
            python_tests,
        )

    os.environ["NS_LOG"] = ""

    if args.kinds:
        path_cmd = os.path.join("utils", test_runner_name + " --print-test-type-list")
        (rc, standard_out, standard_err, et) = run_job_synchronously(
            path_cmd, os.getcwd(), False, False
        )
        print(standard_out)

    if args.list:
        list_items = []
        if ENABLE_TESTS:
            if len(args.constrain):
                path_cmd = os.path.join(
                    "utils",
                    test_runner_name
                    + " --print-test-name-list --print-test-types --test-type=%s"
                    % args.constrain,
                )
            else:
                path_cmd = os.path.join(
                    "utils",
                    test_runner_name + " --print-test-name-list --print-test-types",
                )
            (rc, standard_out, standard_err, et) = run_job_synchronously(
                path_cmd, os.getcwd(), False, False
            )
            if rc != 0:
                print(
                    ("test.py error:  test-runner return code returned {}".format(rc))
                )
                print(
                    (
                        "To debug, try running {}\n".format(
                            "'./ns3 run \"test-runner --print-test-name-list\"'"
                        )
                    )
                )
                return
            if isinstance(standard_out, bytes):
                standard_out = standard_out.decode()
            list_items = standard_out.split("\n")
            list_items.sort()
        print("Test Type    Test Name")
        print("---------    ---------")
        for item in list_items:
            if len(item.strip()):
                print(item)
        examples_sorted = []
        if ENABLE_EXAMPLES:
            examples_sorted = example_names_original
            examples_sorted.sort()
        if ENABLE_PYTHON_BINDINGS:
            python_examples_sorted = []
            for x, y in python_tests:
                if y == "True":
                    python_examples_sorted.append(x)
            python_examples_sorted.sort()
            examples_sorted.extend(python_examples_sorted)
        for item in examples_sorted:
            print("example     ", item)
        print()

    if args.kinds or args.list:
        return

    date_and_time = time.strftime("%Y-%m-%d-%H-%M-%S-CUT", time.gmtime())

    if not os.path.exists(TMP_OUTPUT_DIR):
        os.makedirs(TMP_OUTPUT_DIR)

    testpy_output_dir = os.path.join(TMP_OUTPUT_DIR, date_and_time)

    if not os.path.exists(testpy_output_dir):
        os.makedirs(testpy_output_dir)

    previously_run_tests_to_skip = {"test": [], "example": []}
    if args.rerun_failed:
        previously_run_tests_to_skip = load_previously_successful_tests()

    xml_results_file = os.path.join(TMP_OUTPUT_DIR, f"{date_and_time}-results.xml")
    with open(xml_results_file, "w", encoding="utf-8") as f:
        f.write('<?xml version="1.0"?>\n')
        f.write("<Results>\n")

    single_suite = False

    if len(args.suite):
        path_cmd = os.path.join("utils", test_runner_name + " --print-test-name-list")
        (rc, suites, standard_err, et) = run_job_synchronously(
            path_cmd, os.getcwd(), False, False
        )

        if isinstance(suites, bytes):
            suites = suites.decode()

        suites = suites.replace("\r\n", "\n")
        suites_found = fnmatch.filter(suites.split("\n"), args.suite)

        if not suites_found:
            print(
                "The test suite was not run because an unknown test suite name was requested.",
                file=sys.stderr,
            )
            sys.exit(2)
        elif len(suites_found) == 1:
            single_suite = True

        suites = "\n".join(suites_found)

    elif ENABLE_TESTS and len(args.example) == 0 and len(args.pyexample) == 0:
        if len(args.constrain):
            path_cmd = os.path.join(
                "utils",
                test_runner_name
                + " --print-test-name-list --test-type=%s" % args.constrain,
            )
            (rc, suites, standard_err, et) = run_job_synchronously(
                path_cmd, os.getcwd(), False, False
            )
        else:
            path_cmd = os.path.join(
                "utils", test_runner_name + " --print-test-name-list"
            )
            (rc, suites, standard_err, et) = run_job_synchronously(
                path_cmd, os.getcwd(), False, False
            )
    else:
        suites = ""

    if isinstance(suites, bytes):
        suites = suites.decode()
    suite_list = suites.split("\n")

    if not single_suite and args.constrain != "performance":
        path_cmd = os.path.join(
            "utils",
            test_runner_name + " --print-test-name-list --test-type=%s" % "performance",
        )
        (rc, performance_tests, standard_err, et) = run_job_synchronously(
            path_cmd, os.getcwd(), False, False
        )
        if isinstance(performance_tests, bytes):
            performance_tests = performance_tests.decode()
        performance_test_list = performance_tests.split("\n")

        for performance_test in performance_test_list:
            if performance_test in suite_list:
                suite_list.remove(performance_test)

    input_queue = queue.Queue(0)
    output_queue = queue.Queue(0)

    jobs = 0
    threads = []

    processors = 1

    if sys.platform != "win32":
        if "SC_NPROCESSORS_ONLN" in os.sysconf_names:
            processors = os.sysconf("SC_NPROCESSORS_ONLN")
        else:
            proc = subprocess.Popen(
                "sysctl -n hw.ncpu",
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            stdout_results, stderr_results = proc.communicate()
            stdout_results = stdout_results.decode()
            stderr_results = stderr_results.decode()
            if len(stderr_results) == 0:
                processors = int(stdout_results)
    else:
        processors = os.cpu_count()

    if args.process_limit:
        if processors < args.process_limit:
            print("Using all %s processors" % processors)
        else:
            processors = args.process_limit
            print("Limiting to %s worker processes" % processors)

    for i in range(processors):
        thread = worker_thread(input_queue, output_queue)
        threads.append(thread)
        thread.start()

    total_tests = 0
    skipped_tests = 0
    skipped_testnames = []

    for test in suite_list:
        test = test.strip()
        if len(test):
            job = Job()
            job.set_is_example(False)
            job.set_is_pyexample(False)
            job.set_display_name(test)
            job.set_tmp_file_name(os.path.join(testpy_output_dir, "%s.xml" % test))
            job.set_cwd(os.getcwd())
            job.set_basedir(os.getcwd())
            job.set_tempdir(testpy_output_dir)
            if args.multiple:
                multiple = ""
            else:
                multiple = " --stop-on-failure"
            if len(args.fullness):
                fullness = args.fullness.upper()
                fullness = " --fullness=%s" % fullness
            else:
                fullness = " --fullness=QUICK"

            path_cmd = os.path.join(
                "utils",
                test_runner_name + " --test-name=%s%s%s" % (test, multiple, fullness),
            )

            job.set_shell_command(path_cmd)

            if args.valgrind and test in core_valgrind_skip_tests:
                job.set_is_skip(True)
                job.set_skip_reason("crashes valgrind")

            if args.rerun_failed and test in previously_run_tests_to_skip["test"]:
                job.is_skip = True
                job.set_skip_reason("didn't fail in the previous run")

            if args.verbose:
                print("Queue %s" % test)

            input_queue.put(job)
            jobs = jobs + 1
            total_tests = total_tests + 1

    if len(args.suite) == 0 and len(args.example) == 0 and len(args.pyexample) == 0:
        if len(args.constrain) == 0 or args.constrain == "example":
            if ENABLE_EXAMPLES:
                for name, test, do_run, do_valgrind_run in example_tests:
                    test_name = test.split(" ", 1)[0]
                    test_name = os.path.basename(test_name)
                    test_name = test_name[:-4] if sys.platform == "win32" else test_name

                    if test_name in ns3_runnable_programs_dictionary:
                        if eval(do_run):
                            job = Job()
                            job.set_is_example(True)
                            job.set_is_pyexample(False)
                            job.set_display_name(name)
                            job.set_tmp_file_name("")
                            job.set_cwd(testpy_output_dir)
                            job.set_basedir(os.getcwd())
                            job.set_tempdir(testpy_output_dir)
                            job.set_shell_command(test)
                            job.set_build_path(args.buildpath)

                            if args.valgrind and not eval(do_valgrind_run):
                                job.set_is_skip(True)
                                job.set_skip_reason("skip in valgrind runs")

                            if (
                                args.rerun_failed
                                and name in previously_run_tests_to_skip["example"]
                            ):
                                job.is_skip = True
                                job.set_skip_reason("didn't fail in the previous run")

                            if args.verbose:
                                print("Queue %s" % test)

                            input_queue.put(job)
                            jobs = jobs + 1
                            total_tests = total_tests + 1

    elif len(args.example):
        example_name = "%s%s-%s%s" % (
            APPNAME,
            VERSION,
            args.example,
            BUILD_PROFILE_SUFFIX,
        )

        key_list = []
        for key in ns3_runnable_programs_dictionary:
            key_list.append(key)
        example_name_key_list = fnmatch.filter(key_list, example_name)

        if len(example_name_key_list) == 0:
            print("No example matching the name %s" % args.example)
        else:
            for example_name_iter in example_name_key_list:
                example_path = ns3_runnable_programs_dictionary[example_name_iter]
                example_path = os.path.abspath(example_path)
                job = Job()
                job.set_is_example(True)
                job.set_is_pyexample(False)
                job.set_display_name(example_path)
                job.set_tmp_file_name("")
                job.set_cwd(testpy_output_dir)
                job.set_basedir(os.getcwd())
                job.set_tempdir(testpy_output_dir)
                job.set_shell_command(example_path)
                job.set_build_path(args.buildpath)

                if args.verbose:
                    print("Queue %s" % example_name_iter)

                input_queue.put(job)
                jobs = jobs + 1
                total_tests = total_tests + 1

    if len(args.suite) == 0 and len(args.example) == 0 and len(args.pyexample) == 0:
        if len(args.constrain) == 0 or args.constrain == "pyexample":
            for test, do_run in python_tests:
                test_name = test.split(" ", 1)[0]
                test_name = os.path.basename(test_name)

                if test_name in ns3_runnable_scripts:
                    if eval(do_run):
                        job = Job()
                        job.set_is_example(False)
                        job.set_is_pyexample(True)
                        job.set_display_name(test)
                        job.set_tmp_file_name("")
                        job.set_cwd(testpy_output_dir)
                        job.set_basedir(os.getcwd())
                        job.set_tempdir(testpy_output_dir)
                        job.set_shell_command(test)
                        job.set_build_path("")

                        if args.valgrind:
                            job.set_is_skip(True)
                            job.set_skip_reason("skip in valgrind runs")

                        if not ENABLE_PYTHON_BINDINGS:
                            job.set_is_skip(True)
                            job.set_skip_reason("requires Python bindings")

                        if args.verbose:
                            print("Queue %s" % test)

                        input_queue.put(job)
                        jobs = jobs + 1
                        total_tests = total_tests + 1

    elif len(args.pyexample):
        if not os.path.exists(args.pyexample):
            import glob

            files = glob.glob("./**/%s" % args.pyexample, recursive=True)
            if files:
                args.pyexample = files[0]

        example_name = os.path.basename(args.pyexample)
        if example_name not in ns3_runnable_scripts:
            print("Example %s is not runnable." % example_name)
        elif not os.path.exists(args.pyexample):
            print("Example %s does not exist." % example_name)
        else:
            job = Job()
            job.set_is_pyexample(True)
            job.set_display_name(args.pyexample)
            job.set_tmp_file_name("")
            job.set_cwd(testpy_output_dir)
            job.set_basedir(os.getcwd())
            job.set_tempdir(testpy_output_dir)
            job.set_shell_command(args.pyexample)
            job.set_build_path("")

            if args.verbose:
                print("Queue %s" % args.pyexample)

            input_queue.put(job)
            jobs = jobs + 1
            total_tests = total_tests + 1

    for i in range(processors):
        job = Job()
        job.set_is_break(True)
        input_queue.put(job)

    passed_tests = 0
    failed_tests = 0
    failed_testnames = []
    crashed_tests = 0
    crashed_testnames = []
    valgrind_errors = 0
    valgrind_testnames = []
    failed_jobs = []
    for i in range(jobs):
        job = output_queue.get()
        if job.is_break:
            continue

        if job.is_example or job.is_pyexample:
            kind = "Example"
        else:
            kind = "TestSuite"

        if job.is_skip:
            status = "SKIP"
            status_print = colors.GREY + status + colors.NORMAL
            skipped_tests = skipped_tests + 1
            skipped_testnames.append(job.display_name + (" (%s)" % job.skip_reason))
        else:
            failed_jobs.append(job)
            if job.returncode == 0:
                status = "PASS"
                status_print = colors.GREEN + status + colors.NORMAL
                passed_tests = passed_tests + 1
                failed_jobs.pop()
            elif job.returncode == 1:
                failed_tests = failed_tests + 1
                failed_testnames.append(job.display_name)
                status = "FAIL"
                status_print = colors.RED + status + colors.NORMAL
            elif job.returncode == 2:
                valgrind_errors = valgrind_errors + 1
                valgrind_testnames.append(job.display_name)
                status = "VALGR"
                status_print = colors.CYAN + status + colors.NORMAL
            else:
                crashed_tests = crashed_tests + 1
                crashed_testnames.append(job.display_name)
                status = "CRASH"
                status_print = colors.PINK + status + colors.NORMAL

        print("[%d/%d]" % (i, total_tests), end=" ")
        if args.duration or args.constrain == "performance":
            print(
                "%s (%.3f): %s %s"
                % (status_print, job.elapsed_time, kind, job.display_name)
            )
        else:
            print("%s: %s %s" % (status_print, kind, job.display_name))

        if job.is_example or job.is_pyexample:
            with open(xml_results_file, "a", encoding="utf-8") as f:
                f.write("<Example>\n")
                example_name = "  <Name>%s</Name>\n" % job.display_name
                f.write(example_name)

                if status == "PASS":
                    f.write("  <Result>PASS</Result>\n")
                elif status == "FAIL":
                    f.write("  <Result>FAIL</Result>\n")
                elif status == "VALGR":
                    f.write("  <Result>VALGR</Result>\n")
                elif status == "SKIP":
                    f.write("  <Result>SKIP</Result>\n")
                else:
                    f.write("  <Result>CRASH</Result>\n")

                f.write('  <Time real="%.3f"/>\n' % job.elapsed_time)
                f.write("</Example>\n")

        else:
            if job.is_skip:
                with open(xml_results_file, "a", encoding="utf-8") as f:
                    f.write("<Test>\n")
                    f.write("  <Name>%s</Name>\n" % job.display_name)
                    f.write("  <Result>SKIP</Result>\n")
                    f.write("  <Reason>%s</Reason>\n" % job.skip_reason)
                    f.write("</Test>\n")
            else:
                failed_jobs.append(job)
                if job.returncode == 0 or job.returncode == 1 or job.returncode == 2:
                    with (
                        open(xml_results_file, "a", encoding="utf-8") as f_to,
                        open(job.tmp_file_name, encoding="utf-8") as f_from,
                    ):
                        contents = f_from.read()
                        if status == "VALGR":
                            pre = contents.find("<Result>") + len("<Result>")
                            post = contents.find("</Result>")
                            contents = contents[:pre] + "VALGR" + contents[post:]
                        f_to.write(contents)
                        et = ET.parse(job.tmp_file_name)
                        if et.find("Result").text in ["PASS", "SKIP"]:
                            failed_jobs.pop()
                else:
                    with open(xml_results_file, "a", encoding="utf-8") as f:
                        f.write("<Test>\n")
                        f.write("  <Name>%s</Name>\n" % job.display_name)
                        f.write("  <Result>CRASH</Result>\n")
                        f.write("</Test>\n")

    for thread in threads:
        thread.join()

    with open(xml_results_file, "a", encoding="utf-8") as f:
        f.write("</Results>\n")

    print(
        "%d of %d tests passed (%d passed, %d skipped, %d failed, %d crashed, %d valgrind errors)"
        % (
            passed_tests,
            total_tests,
            passed_tests,
            skipped_tests,
            failed_tests,
            crashed_tests,
            valgrind_errors,
        )
    )
    if skipped_testnames:
        skipped_testnames.sort()
        print(
            "List of SKIPped tests:\n    %s"
            % "\n    ".join(map(str, skipped_testnames))
        )
    if failed_testnames:
        failed_testnames.sort()
        print(
            "List of FAILed tests:\n    %s" % "\n    ".join(map(str, failed_testnames))
        )
    if crashed_testnames:
        crashed_testnames.sort()
        print(
            "List of CRASHed tests:\n    %s"
            % "\n    ".join(map(str, crashed_testnames))
        )
    if valgrind_testnames:
        valgrind_testnames.sort()
        print(
            "List of VALGR failures:\n    %s"
            % "\n    ".join(map(str, valgrind_testnames))
        )

    if failed_jobs and args.verbose_failed:
        for job in failed_jobs:
            if job.standard_out or job.standard_err:
                job_type = (
                    "example" if (job.is_example or job.is_pyexample) else "test suite"
                )
                print(
                    f"===================== Begin of {job_type} '{job.display_name}' stdout ====================="
                )
                print(job.standard_out)
                print(
                    f"===================== Begin of {job_type} '{job.display_name}' stderr ====================="
                )
                print(job.standard_err)
                print(
                    f"===================== End of {job_type} '{job.display_name}' =============================="
                )
    if len(args.html) + len(args.text) + len(args.xml):
        print()

    if len(args.html):
        translate_to_html(xml_results_file, args.html)

    if len(args.text):
        translate_to_text(xml_results_file, args.text)

    if len(args.xml):
        xml_file = args.xml + (".xml" if ".xml" not in args.xml else "")
        print("Writing results to xml file %s..." % xml_file, end="")
        shutil.copyfile(xml_results_file, xml_file)
        print("done.")

    if not ENABLE_TESTS or not ENABLE_EXAMPLES:
        print()
        if not ENABLE_TESTS:
            print("***  Note: ns-3 tests are currently disabled. Enable them by adding")
            print(
                '***  "--enable-tests" to ./ns3 configure or modifying your .ns3rc file.'
            )
            print()
        if not ENABLE_EXAMPLES:
            print(
                "***  Note: ns-3 examples are currently disabled. Enable them by adding"
            )
            print(
                '***  "--enable-examples" to ./ns3 configure or modifying your .ns3rc file.'
            )
            print()

    if args.valgrind and not VALGRIND_FOUND:
        print()
        print(
            "***  Note: you are trying to use valgrind, but valgrind could not be found"
        )
        print("***  on your machine.  All tests and examples will crash or be skipped.")
        print()

    if not args.retain:
        shutil.rmtree(testpy_output_dir)

    if passed_tests + skipped_tests == total_tests:
        return 0
    else:
        return 1


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-b",
        "--buildpath",
        action="store",
        type=str,
        default="",
        help="specify the path where ns-3 was built (defaults to the build directory for the current variant)",
    )

    parser.add_argument(
        "-c",
        "--constrain",
        action="store",
        type=str,
        default="",
        help="constrain the test-runner by kind of test",
    )

    parser.add_argument(
        "-d",
        "--duration",
        action="store_true",
        default=False,
        help="print the duration of each test suite and example",
    )

    parser.add_argument(
        "-e",
        "--example",
        action="store",
        type=str,
        default="",
        help="specify a single example to run (no relative path is needed)",
    )

    parser.add_argument(
        "-u",
        "--update-data",
        action="store_true",
        default=False,
        help="If examples use reference data files, get them to re-generate them",
    )

    parser.add_argument(
        "-f",
        "--fullness",
        action="store",
        type=str,
        default="QUICK",
        choices=["QUICK", "EXTENSIVE", "TAKES_FOREVER"],
        help="choose the duration of tests to run: QUICK, EXTENSIVE, or TAKES_FOREVER, where EXTENSIVE includes QUICK and TAKES_FOREVER includes QUICK and EXTENSIVE (only QUICK tests are run by default)",
    )

    parser.add_argument(
        "-g",
        "--grind",
        action="store_true",
        dest="valgrind",
        default=False,
        help="run the test suites and examples using valgrind",
    )

    parser.add_argument(
        "-k",
        "--kinds",
        action="store_true",
        default=False,
        help="print the kinds of tests available",
    )

    parser.add_argument(
        "-l",
        "--list",
        action="store_true",
        default=False,
        help="print the list of known tests",
    )

    parser.add_argument(
        "-m",
        "--multiple",
        action="store_true",
        default=False,
        help="report multiple failures from test suites and test cases",
    )

    parser.add_argument(
        "-n",
        "--no-build",
        action="store_true",
        default=False,
        help="do not build before starting testing",
    )

    parser.add_argument(
        "-p",
        "--pyexample",
        action="store",
        type=str,
        default="",
        help="specify a single python example to run (with relative path)",
    )

    parser.add_argument(
        "-r",
        "--retain",
        action="store_true",
        default=False,
        help="retain all temporary files (which are normally deleted)",
    )

    parser.add_argument(
        "-s",
        "--suite",
        action="store",
        type=str,
        default="",
        help="specify a single test suite to run",
    )

    parser.add_argument(
        "-t",
        "--text",
        action="store",
        type=str,
        default="",
        metavar="TEXT-FILE",
        help="write detailed test results into TEXT-FILE.txt",
    )

    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        default=False,
        help="print progress and informational messages",
    )

    parser.add_argument(
        "--verbose-failed",
        action="store_true",
        default=False,
        help="print progress and informational messages for failed jobs",
    )

    parser.add_argument(
        "-w",
        "--web",
        "--html",
        action="store",
        type=str,
        dest="html",
        default="",
        metavar="HTML-FILE",
        help="write detailed test results into HTML-FILE.html",
    )

    parser.add_argument(
        "-x",
        "--xml",
        action="store",
        type=str,
        default="",
        metavar="XML-FILE",
        help="write detailed test results into XML-FILE.xml",
    )

    parser.add_argument(
        "--nocolor",
        action="store_true",
        default=False,
        help="do not use colors in the standard output",
    )

    parser.add_argument(
        "--jobs",
        action="store",
        type=int,
        dest="process_limit",
        default=0,
        help="limit number of worker threads",
    )

    parser.add_argument(
        "--rerun-failed",
        action="store_true",
        dest="rerun_failed",
        default=False,
        help="rerun failed tests",
    )

    global args
    args = parser.parse_args()
    signal.signal(signal.SIGINT, sigint_hook)

    envcolor = os.environ.get("NOCOLOR", "") and "no" or "auto" or "yes"

    if args.nocolor or envcolor == "no":
        colors_lst["USE"] = False

    return run_tests()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
