#! /usr/bin/env python3

"""!
Test suite for the ns3 wrapper script
"""

import glob
import os
import re
import shutil
import subprocess
import sys
import unittest
from functools import partial

ns3_path = os.path.dirname(os.path.abspath(os.sep.join([__file__, "../../"])))
ns3_lock_filename = os.path.join(ns3_path, ".lock-ns3_%s_build" % sys.platform)
ns3_script = os.sep.join([ns3_path, "ns3"])
ns3rc_script = os.sep.join([ns3_path, ".ns3rc"])
usual_outdir = os.sep.join([ns3_path, "build"])
usual_lib_outdir = os.sep.join([usual_outdir, "lib"])

os.chdir(ns3_path)

num_threads = max(1, os.cpu_count() - 1)
cmake_build_project_command = "cmake --build . -j".format(ns3_path=ns3_path)
cmake_build_target_command = partial(
    "cmake --build . -j {jobs} --target {target}".format, jobs=num_threads
)
win32 = sys.platform == "win32"
platform_makefiles = "MinGW Makefiles" if win32 else "Unix Makefiles"
ext = ".exe" if win32 else ""


def run_ns3(args, env=None, generator=platform_makefiles):
    """!
    Runs the ns3 wrapper script with arguments
    @param args: string containing arguments that will get split before calling ns3
    @param env: environment variables dictionary
    @param generator: CMake generator
    @return tuple containing (error code, stdout and stderr)
    """
    if "clean" in args:
        possible_leftovers = ["contrib/borked", "contrib/calibre"]
        for leftover in possible_leftovers:
            if os.path.exists(leftover):
                shutil.rmtree(leftover, ignore_errors=True)
    if " -G " in args:
        args = args.format(generator=generator)
    if env is None:
        env = {}
    env["CLICOLOR"] = "0"
    return run_program(ns3_script, args, python=True, env=env)


def run_program(program, args, python=False, cwd=ns3_path, env=None):
    """!
    Runs a program with the given arguments and returns a tuple containing (error code, stdout and stderr)
    @param program: program to execute (or python script)
    @param args: string containing arguments that will get split before calling the program
    @param python: flag indicating whether the program is a python script
    @param cwd: the working directory used that will be the root folder for the execution
    @param env: environment variables dictionary
    @return tuple containing (error code, stdout and stderr)
    """
    if type(args) != str:
        raise Exception("args should be a string")

    if python:
        arguments = [sys.executable, program]
    else:
        arguments = [program]

    if args != "":
        arguments.extend(re.findall('(?:".*?"|\S)+', args))  # noqa

    for i in range(len(arguments)):
        arguments[i] = arguments[i].replace('"', "")

    current_env = os.environ.copy()

    if env:
        current_env.update(env)

    ret = subprocess.run(
        arguments,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=cwd,
        env=current_env,
    )
    return (
        ret.returncode,
        ret.stdout.decode(sys.stdout.encoding),
        ret.stderr.decode(sys.stderr.encoding),
    )


def get_programs_list():
    """!
    Extracts the programs list from .lock-ns3
    @return list of programs.
    """
    values = {}
    with open(ns3_lock_filename, encoding="utf-8") as f:
        exec(f.read(), globals(), values)

    programs_list = values["ns3_runnable_programs"]

    if win32:
        programs_list = list(map(lambda x: x + ext, programs_list))
    return programs_list


def get_libraries_list(lib_outdir=usual_lib_outdir):
    """!
    Gets a list of built libraries
    @param lib_outdir: path containing libraries
    @return list of built libraries.
    """
    libraries = glob.glob(lib_outdir + "/*", recursive=True)
    return list(filter(lambda x: "scratch-nested-subdir-lib" not in x, libraries))


def get_headers_list(outdir=usual_outdir):
    """!
    Gets a list of header files
    @param outdir: path containing headers
    @return list of headers.
    """
    return glob.glob(outdir + "/**/*.h", recursive=True)


def read_lock_entry(entry):
    """!
    Read interesting entries from the .lock-ns3 file
    @param entry: entry to read from .lock-ns3
    @return value of the requested entry.
    """
    values = {}
    with open(ns3_lock_filename, encoding="utf-8") as f:
        exec(f.read(), globals(), values)
    return values.get(entry, None)


def get_test_enabled():
    """!
    Check if tests are enabled in the .lock-ns3
    @return bool.
    """
    return read_lock_entry("ENABLE_TESTS")


def get_enabled_modules():
    """
    Check if tests are enabled in the .lock-ns3
    @return list of enabled modules (prefixed with 'ns3-').
    """
    return read_lock_entry("NS3_ENABLED_MODULES")


class DockerContainerManager:
    """!
    Python-on-whales wrapper for Docker-based ns-3 tests
    """

    def __init__(
        self, currentTestCase: unittest.TestCase, containerName: str = "ubuntu:latest"
    ):
        """!
        Create and start container with containerName in the current ns-3 directory
        @param self: the current DockerContainerManager instance
        @param currentTestCase: the test case instance creating the DockerContainerManager
        @param containerName: name of the container image to be used
        """
        global DockerException
        try:
            from python_on_whales import docker
            from python_on_whales.exceptions import DockerException
        except ModuleNotFoundError:
            docker = None  # noqa
            DockerException = None  # noqa
            currentTestCase.skipTest("python-on-whales was not found")

        with open(os.path.expanduser("~/.bashrc"), "r", encoding="utf-8") as f:
            docker_settings = re.findall("(DOCKER_.*=.*)", f.read())
            for setting in docker_settings:
                key, value = setting.split("=")
                os.environ[key] = value
            del docker_settings, setting, key, value

        self.container = docker.run(
            containerName,
            interactive=True,
            detach=True,
            tty=False,
            volumes=[(ns3_path, "/ns-3-dev")],
        )

        def split_exec(docker_container, cmd):
            return docker_container._execute(cmd.split(), workdir="/ns-3-dev")

        self.container._execute = self.container.execute
        self.container.execute = partial(split_exec, self.container)

    def __enter__(self):
        """!
        Return the managed container when entiring the block "with DockerContainerManager() as container"
        @param self: the current DockerContainerManager instance
        @return container managed by DockerContainerManager.
        """
        return self.container

    def __exit__(self, exc_type, exc_val, exc_tb):
        """!
        Clean up the managed container at the end of the block "with DockerContainerManager() as container"
        @param self: the current DockerContainerManager instance
        @param exc_type: unused parameter
        @param exc_val: unused parameter
        @param exc_tb: unused parameter
        @return None
        """
        self.container.stop()
        self.container.remove()


class NS3UnusedSourcesTestCase(unittest.TestCase):
    """!
    ns-3 tests related to checking if source files were left behind, not being used by CMake
    """

    directory_and_files = {}

    def setUp(self):
        """!
        Scan all C++ source files and add them to a list based on their path
        @return None
        """
        for root, dirs, files in os.walk(ns3_path):
            if "gitlab-ci-local" in root:
                continue
            for name in files:
                if name.endswith(".cc"):
                    path = os.path.join(root, name)
                    directory = os.path.dirname(path)
                    if directory not in self.directory_and_files:
                        self.directory_and_files[directory] = []
                    self.directory_and_files[directory].append(path)

    def test_01_UnusedExampleSources(self):
        """!
        Test if all example source files are being used in their respective CMakeLists.txt
        @return None
        """
        unused_sources = set()
        for example_directory in self.directory_and_files.keys():
            if os.sep + "examples" not in example_directory:
                continue

            with open(
                os.path.join(example_directory, "CMakeLists.txt"), "r", encoding="utf-8"
            ) as f:
                cmake_contents = f.read()

            for file in self.directory_and_files[example_directory]:
                if os.path.basename(file).replace(".cc", "") not in cmake_contents:
                    unused_sources.add(file)

        self.assertListEqual([], list(unused_sources))

    def test_02_UnusedModuleSources(self):
        """!
        Test if all module source files are being used in their respective CMakeLists.txt
        @return None
        """
        unused_sources = set()
        for directory in self.directory_and_files.keys():
            is_not_module = not ("src" in directory or "contrib" in directory)
            is_example = os.sep + "examples" in directory
            is_bindings = os.sep + "bindings" in directory

            if is_not_module or is_bindings or is_example:
                continue

            cmake_path = os.path.join(directory, "CMakeLists.txt")
            while not os.path.exists(cmake_path):
                parent_directory = os.path.dirname(os.path.dirname(cmake_path))
                cmake_path = os.path.join(
                    parent_directory, os.path.basename(cmake_path)
                )

            with open(cmake_path, "r", encoding="utf-8") as f:
                cmake_contents = f.read()

            for file in self.directory_and_files[directory]:
                if os.path.basename(file) not in cmake_contents:
                    unused_sources.add(file)

        exceptions = [
            "win32-system-wall-clock-ms.cc",
        ]
        for exception in exceptions:
            for unused_source in unused_sources:
                if os.path.basename(unused_source) == exception:
                    unused_sources.remove(unused_source)
                    break

        self.assertListEqual([], list(unused_sources))

    def test_03_UnusedUtilsSources(self):
        """!
        Test if all utils source files are being used in their respective CMakeLists.txt
        @return None
        """
        unused_sources = set()
        for directory in self.directory_and_files.keys():
            is_module = "src" in directory or "contrib" in directory
            if os.sep + "utils" not in directory or is_module:
                continue

            cmake_path = os.path.join(directory, "CMakeLists.txt")
            while not os.path.exists(cmake_path):
                parent_directory = os.path.dirname(os.path.dirname(cmake_path))
                cmake_path = os.path.join(
                    parent_directory, os.path.basename(cmake_path)
                )

            with open(cmake_path, "r", encoding="utf-8") as f:
                cmake_contents = f.read()

            for file in self.directory_and_files[directory]:
                if os.path.basename(file) not in cmake_contents:
                    unused_sources.add(file)

        self.assertListEqual([], list(unused_sources))


class NS3DependenciesTestCase(unittest.TestCase):
    """!
    ns-3 tests related to dependencies
    """

    def test_01_CheckIfIncludedHeadersMatchLinkedModules(self):
        """!
        Checks if headers from different modules (src/A, contrib/B) that are included by
        the current module (src/C) source files correspond to the list of linked modules
        LIBNAME C
        LIBRARIES_TO_LINK A (missing B)
        @return None
        """
        modules = {}
        headers_to_modules = {}
        module_paths = glob.glob(ns3_path + "/src/*/") + glob.glob(
            ns3_path + "/contrib/*/"
        )

        for path in module_paths:
            cmake_path = os.path.join(path, "CMakeLists.txt")
            with open(cmake_path, "r", encoding="utf-8") as f:
                cmake_contents = f.readlines()

            module_name = os.path.relpath(path, ns3_path)
            module_name_nodir = module_name.replace("src/", "").replace("contrib/", "")
            modules[module_name_nodir] = {
                "sources": set(),
                "headers": set(),
                "libraries": set(),
                "included_headers": set(),
                "included_libraries": set(),
            }

            for line in cmake_contents:
                base_name = os.path.basename(line[:-1])
                if not os.path.exists(os.path.join(path, line.strip())):
                    continue

                if ".h" in line:
                    modules[module_name_nodir]["headers"].add(base_name)
                    modules[module_name_nodir]["sources"].add(base_name)

                    headers_to_modules[base_name] = module_name_nodir

                if ".cc" in line:
                    modules[module_name_nodir]["sources"].add(base_name)

                if ".cc" in line or ".h" in line:
                    source_file = os.path.join(ns3_path, module_name, line.strip())
                    with open(source_file, "r", encoding="utf-8") as f:
                        source_contents = f.read()
                    modules[module_name_nodir]["included_headers"].update(
                        map(
                            lambda x: x.replace("ns3/", ""),
                            re.findall('#include.*["|<](.*)["|>]', source_contents),
                        )
                    )
                    continue

            modules[module_name_nodir]["libraries"].update(
                re.findall("\\${lib(.*)}", "".join(cmake_contents))
            )

        all_project_headers = set(headers_to_modules.keys())

        sys.stderr.flush()
        print(file=sys.stderr)
        for module in sorted(modules):
            external_headers = modules[module]["included_headers"].difference(
                all_project_headers
            )
            project_headers_included = modules[module]["included_headers"].difference(
                external_headers
            )
            modules[module]["included_libraries"] = set(
                [headers_to_modules[x] for x in project_headers_included]
            ).difference({module})

            diff = modules[module]["included_libraries"].difference(
                modules[module]["libraries"]
            )
            if len(diff) > 0:
                print(
                    "Module %s includes modules that are not linked: %s"
                    % (module, ", ".join(list(diff))),
                    file=sys.stderr,
                )
                sys.stderr.flush()
        self.assertTrue(True)


class NS3StyleTestCase(unittest.TestCase):
    """!
    ns-3 tests to check if the source code, whitespaces and CMake formatting
    are according to the coding style
    """

    starting_diff = None
    repo = None

    def setUp(self) -> None:
        """!
        Import GitRepo and load the original diff state of the repository before the tests
        @return None
        """
        if not NS3StyleTestCase.starting_diff:
            if shutil.which("git") is None:
                self.skipTest("Git is not available")

            try:
                from git import Repo  # noqa
                import git.exc  # noqa
            except ImportError:
                self.skipTest("GitPython is not available")

            try:
                repo = Repo(ns3_path)  # noqa
            except git.exc.InvalidGitRepositoryError:  # noqa
                self.skipTest("ns-3 directory does not contain a .git directory")

            hcommit = repo.head.commit  # noqa
            NS3StyleTestCase.starting_diff = hcommit.diff(None)
            NS3StyleTestCase.repo = repo

        if NS3StyleTestCase.starting_diff is None:
            self.skipTest("Unmet dependencies")

    def test_01_CheckCMakeFormat(self):
        """!
        Check if there is any difference between tracked file after
        applying cmake-format
        @return None
        """

        for required_program in ["cmake", "cmake-format"]:
            if shutil.which(required_program) is None:
                self.skipTest("%s was not found" % required_program)

        return_code, stdout, stderr = run_ns3("configure")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("build cmake-format")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("clean")
        self.assertEqual(return_code, 0)

        new_diff = NS3StyleTestCase.repo.head.commit.diff(None)
        self.assertEqual(NS3StyleTestCase.starting_diff, new_diff)


class NS3CommonSettingsTestCase(unittest.TestCase):
    """!
    ns3 tests related to generic options
    """

    def setUp(self):
        """!
        Clean configuration/build artifacts before common commands
        @return None
        """
        super().setUp()
        run_ns3("clean")

    def test_01_NoOption(self):
        """!
        Test not passing any arguments to
        @return None
        """
        return_code, stdout, stderr = run_ns3("")
        self.assertEqual(return_code, 1)
        self.assertIn("You need to configure ns-3 first: try ./ns3 configure", stdout)

    def test_02_NoTaskLines(self):
        """!
        Test only passing --quiet argument to ns3
        @return None
        """
        return_code, stdout, stderr = run_ns3("--quiet")
        self.assertEqual(return_code, 1)
        self.assertIn("You need to configure ns-3 first: try ./ns3 configure", stdout)

    def test_03_CheckConfig(self):
        """!
        Test only passing 'show config' argument to ns3
        @return None
        """
        return_code, stdout, stderr = run_ns3("show config")
        self.assertEqual(return_code, 1)
        self.assertIn("You need to configure ns-3 first: try ./ns3 configure", stdout)

    def test_04_CheckProfile(self):
        """!
        Test only passing 'show profile' argument to ns3
        @return None
        """
        return_code, stdout, stderr = run_ns3("show profile")
        self.assertEqual(return_code, 1)
        self.assertIn("You need to configure ns-3 first: try ./ns3 configure", stdout)

    def test_05_CheckVersion(self):
        """!
        Test only passing 'show version' argument to ns3
        @return None
        """
        return_code, stdout, stderr = run_ns3("show version")
        self.assertEqual(return_code, 1)
        self.assertIn("You need to configure ns-3 first: try ./ns3 configure", stdout)


class NS3ConfigureBuildProfileTestCase(unittest.TestCase):
    """!
    ns3 tests related to build profiles
    """

    def setUp(self):
        """!
        Clean configuration/build artifacts before testing configuration settings
        @return None
        """
        super().setUp()
        run_ns3("clean")

    def test_01_Debug(self):
        """!
        Test the debug build
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" -d debug --enable-verbose'
        )
        self.assertEqual(return_code, 0)
        self.assertIn("Build profile                 : debug", stdout)
        self.assertIn("Build files have been written to", stdout)

        return_code, stdout, stderr = run_ns3("build core")
        self.assertEqual(return_code, 0)
        self.assertIn("Built target libcore", stdout)

        libraries = get_libraries_list()
        self.assertGreater(len(libraries), 0)
        self.assertIn("core-debug", libraries[0])

    def test_02_Release(self):
        """!
        Test the release build
        @return None
        """
        return_code, stdout, stderr = run_ns3('configure -G "{generator}" -d release')
        self.assertEqual(return_code, 0)
        self.assertIn("Build profile                 : release", stdout)
        self.assertIn("Build files have been written to", stdout)

    def test_03_Optimized(self):
        """!
        Test the optimized build
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" -d optimized --enable-verbose'
        )
        self.assertEqual(return_code, 0)
        self.assertIn("Build profile                 : optimized", stdout)
        self.assertIn("Build files have been written to", stdout)

        return_code, stdout, stderr = run_ns3("build core")
        self.assertEqual(return_code, 0)
        self.assertIn("Built target libcore", stdout)

        libraries = get_libraries_list()
        self.assertGreater(len(libraries), 0)
        self.assertIn("core-optimized", libraries[0])

    def test_04_Typo(self):
        """!
        Test a build type with a typo
        @return None
        """
        return_code, stdout, stderr = run_ns3('configure -G "{generator}" -d Optimized')
        self.assertEqual(return_code, 2)
        self.assertIn("invalid choice: 'Optimized'", stderr)

    def test_05_TYPO(self):
        """!
        Test a build type with another typo
        @return None
        """
        return_code, stdout, stderr = run_ns3('configure -G "{generator}" -d OPTIMIZED')
        self.assertEqual(return_code, 2)
        self.assertIn("invalid choice: 'OPTIMIZED'", stderr)

    def test_06_OverwriteDefaultSettings(self):
        """!
        Replace settings set by default (e.g. ASSERT/LOGs enabled in debug builds and disabled in default ones)
        @return None
        """
        return_code, _, _ = run_ns3("clean")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --dry-run -d debug'
        )
        self.assertEqual(return_code, 0)
        self.assertIn(
            "-DCMAKE_BUILD_TYPE=debug -DNS3_ASSERT=ON -DNS3_LOG=ON -DNS3_WARNINGS_AS_ERRORS=ON -DNS3_NATIVE_OPTIMIZATIONS=OFF",
            stdout,
        )

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --dry-run -d debug --disable-asserts --disable-logs --disable-werror'
        )
        self.assertEqual(return_code, 0)
        self.assertIn(
            "-DCMAKE_BUILD_TYPE=debug -DNS3_NATIVE_OPTIMIZATIONS=OFF -DNS3_ASSERT=OFF -DNS3_LOG=OFF -DNS3_WARNINGS_AS_ERRORS=OFF",
            stdout,
        )

        return_code, stdout, stderr = run_ns3('configure -G "{generator}" --dry-run')
        self.assertEqual(return_code, 0)
        self.assertIn(
            "-DCMAKE_BUILD_TYPE=default -DNS3_ASSERT=ON -DNS3_LOG=ON -DNS3_WARNINGS_AS_ERRORS=OFF -DNS3_NATIVE_OPTIMIZATIONS=OFF",
            stdout,
        )

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --dry-run --enable-asserts --enable-logs --enable-werror'
        )
        self.assertEqual(return_code, 0)
        self.assertIn(
            "-DCMAKE_BUILD_TYPE=default -DNS3_ASSERT=ON -DNS3_LOG=ON -DNS3_NATIVE_OPTIMIZATIONS=OFF -DNS3_ASSERT=ON -DNS3_LOG=ON -DNS3_WARNINGS_AS_ERRORS=ON",
            stdout,
        )


class NS3BaseTestCase(unittest.TestCase):
    """!
    Generic test case with basic function inherited by more complex tests.
    """

    def config_ok(self, return_code, stdout):
        """!
        Check if configuration for release mode worked normally
        @param return_code: return code from CMake
        @param stdout: output from CMake.
        @return None
        """
        self.assertEqual(return_code, 0)
        self.assertIn("Build profile                 : release", stdout)
        self.assertIn("Build files have been written to", stdout)

    def setUp(self):
        """!
        Clean configuration/build artifacts before testing configuration and build settings
        After configuring the build as release,
        check if configuration worked and check expected output files.
        @return None
        """
        super().setUp()

        if os.path.exists(ns3rc_script):
            os.remove(ns3rc_script)

        run_ns3("clean")
        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" -d release --enable-verbose'
        )
        self.config_ok(return_code, stdout)

        self.assertTrue(os.path.exists(ns3_lock_filename))
        self.ns3_executables = get_programs_list()

        self.assertTrue(os.path.exists(ns3_lock_filename))
        self.ns3_modules = get_enabled_modules()


class NS3ConfigureTestCase(NS3BaseTestCase):
    """!
    Test ns3 configuration options
    """

    def setUp(self):
        """!
        Reuse cleaning/release configuration from NS3BaseTestCase if flag is cleaned
        @return None
        """
        super().setUp()

    def test_01_Examples(self):
        """!
        Test enabling and disabling examples
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-examples'
        )

        self.config_ok(return_code, stdout)

        self.assertGreater(len(get_programs_list()), len(self.ns3_executables))

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --disable-examples'
        )

        self.config_ok(return_code, stdout)

        self.assertEqual(len(get_programs_list()), len(self.ns3_executables))

    def test_02_Tests(self):
        """!
        Test enabling and disabling tests
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-tests'
        )
        self.config_ok(return_code, stdout)

        return_code, stdout, stderr = run_ns3("build core-test")

        self.assertEqual(return_code, 0)
        self.assertIn("Built target libcore-test", stdout)

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --disable-tests'
        )
        self.config_ok(return_code, stdout)

        return_code, stdout, stderr = run_ns3("build core-test")

        self.assertEqual(return_code, 1)
        self.assertIn("Target to build does not exist: core-test", stdout)

    def test_03_EnableModules(self):
        """!
        Test enabling specific modules
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --enable-modules='network;wifi'"
        )
        self.config_ok(return_code, stdout)

        enabled_modules = get_enabled_modules()
        self.assertLess(len(get_enabled_modules()), len(self.ns3_modules))
        self.assertIn("ns3-network", enabled_modules)
        self.assertIn("ns3-wifi", enabled_modules)

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --enable-modules='core'"
        )
        self.config_ok(return_code, stdout)
        self.assertIn("ns3-core", get_enabled_modules())

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --enable-modules=''"
        )
        self.config_ok(return_code, stdout)

        self.assertEqual(len(get_enabled_modules()), len(self.ns3_modules))

    def test_04_DisableModules(self):
        """!
        Test disabling specific modules
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --disable-modules='lte;wimax'"
        )
        self.config_ok(return_code, stdout)

        enabled_modules = get_enabled_modules()
        self.assertLess(len(enabled_modules), len(self.ns3_modules))
        self.assertNotIn("ns3-lte", enabled_modules)
        self.assertNotIn("ns3-wimax", enabled_modules)

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --disable-modules=''"
        )
        self.config_ok(return_code, stdout)

        self.assertEqual(len(get_enabled_modules()), len(self.ns3_modules))

    def test_05_EnableModulesComma(self):
        """!
        Test enabling comma-separated (waf-style) examples
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --enable-modules='network,wifi'"
        )
        self.config_ok(return_code, stdout)

        enabled_modules = get_enabled_modules()
        self.assertLess(len(get_enabled_modules()), len(self.ns3_modules))
        self.assertIn("ns3-network", enabled_modules)
        self.assertIn("ns3-wifi", enabled_modules)

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --enable-modules=''"
        )
        self.config_ok(return_code, stdout)

        self.assertEqual(len(get_enabled_modules()), len(self.ns3_modules))

    def test_06_DisableModulesComma(self):
        """!
        Test disabling comma-separated (waf-style) examples
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --disable-modules='lte,mpi'"
        )
        self.config_ok(return_code, stdout)

        enabled_modules = get_enabled_modules()
        self.assertLess(len(enabled_modules), len(self.ns3_modules))
        self.assertNotIn("ns3-lte", enabled_modules)
        self.assertNotIn("ns3-mpi", enabled_modules)

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --disable-modules=''"
        )
        self.config_ok(return_code, stdout)

        self.assertEqual(len(get_enabled_modules()), len(self.ns3_modules))

    def test_07_Ns3rc(self):
        """!
        Test loading settings from the ns3rc config file
        @return None
        """

        class ns3rc_str:  # noqa
            ns3rc_python_template = "# ! /usr/bin/env python\
                                 \
                                 # A list of the modules that will be enabled when ns-3 is run.\
                                 # Modules that depend on the listed modules will be enabled also.\
                                 #\
                                 # All modules can be enabled by choosing 'all_modules'.\
                                 modules_enabled = [{modules}]\
                                 \
                                 # Set this equal to true if you want examples to be run.\
                                 examples_enabled = {examples}\
                                 \
                                 # Set this equal to true if you want tests to be run.\
                                 tests_enabled = {tests}\
                                 "

            ns3rc_cmake_template = "set(ns3rc_tests_enabled {tests})\
                                    \nset(ns3rc_examples_enabled {examples})\
                                    \nset(ns3rc_enabled_modules {modules})\
                                    "

            ns3rc_templates = {
                "python": ns3rc_python_template,
                "cmake": ns3rc_cmake_template,
            }

            def __init__(self, type_ns3rc):
                self.type = type_ns3rc

            def format(self, **args):
                if self.type == "cmake":
                    args["modules"] = (
                        args["modules"]
                        .replace("'", "")
                        .replace('"', "")
                        .replace(",", " ")
                    )
                    args["examples"] = "ON" if args["examples"] == "True" else "OFF"
                    args["tests"] = "ON" if args["tests"] == "True" else "OFF"

                formatted_string = ns3rc_str.ns3rc_templates[self.type].format(**args)

                return formatted_string

            @staticmethod
            def types():
                return ns3rc_str.ns3rc_templates.keys()

        for ns3rc_type in ns3rc_str.types():
            ns3rc_template = ns3rc_str(ns3rc_type)

            with open(ns3rc_script, "w", encoding="utf-8") as f:
                f.write(
                    ns3rc_template.format(
                        modules="'lte'", examples="False", tests="True"
                    )
                )

            return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
            self.config_ok(return_code, stdout)

            enabled_modules = get_enabled_modules()
            self.assertLess(len(get_enabled_modules()), len(self.ns3_modules))
            self.assertIn("ns3-lte", enabled_modules)
            self.assertTrue(get_test_enabled())
            self.assertLessEqual(len(get_programs_list()), len(self.ns3_executables))

            with open(ns3rc_script, "w", encoding="utf-8") as f:
                f.write(
                    ns3rc_template.format(
                        modules="'wifi'", examples="True", tests="False"
                    )
                )

            return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
            self.config_ok(return_code, stdout)

            enabled_modules = get_enabled_modules()
            self.assertLess(len(get_enabled_modules()), len(self.ns3_modules))
            self.assertIn("ns3-wifi", enabled_modules)
            self.assertFalse(get_test_enabled())
            self.assertGreater(len(get_programs_list()), len(self.ns3_executables))

            with open(ns3rc_script, "w", encoding="utf-8") as f:
                f.write(
                    ns3rc_template.format(
                        modules="'core','network'", examples="True", tests="False"
                    )
                )

            return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
            self.config_ok(return_code, stdout)

            enabled_modules = get_enabled_modules()
            self.assertLess(len(get_enabled_modules()), len(self.ns3_modules))
            self.assertIn("ns3-core", enabled_modules)
            self.assertIn("ns3-network", enabled_modules)
            self.assertFalse(get_test_enabled())
            self.assertGreater(len(get_programs_list()), len(self.ns3_executables))

            with open(ns3rc_script, "w", encoding="utf-8") as f:
                if ns3rc_type == "python":
                    f.write(
                        ns3rc_template.format(
                            modules="""'core', #comment
                    'lte',
                    #comment2,
                    #comment3
                    'network', 'internet','wimax'""",
                            examples="True",
                            tests="True",
                        )
                    )
                else:
                    f.write(
                        ns3rc_template.format(
                            modules="'core', 'lte', 'network', 'internet', 'wimax'",
                            examples="True",
                            tests="True",
                        )
                    )
            return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
            self.config_ok(return_code, stdout)

            enabled_modules = get_enabled_modules()
            self.assertLess(len(get_enabled_modules()), len(self.ns3_modules))
            self.assertIn("ns3-core", enabled_modules)
            self.assertIn("ns3-internet", enabled_modules)
            self.assertIn("ns3-lte", enabled_modules)
            self.assertIn("ns3-wimax", enabled_modules)
            self.assertTrue(get_test_enabled())
            self.assertGreater(len(get_programs_list()), len(self.ns3_executables))

            os.remove(ns3rc_script)

            return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
            self.config_ok(return_code, stdout)

            self.assertEqual(len(get_enabled_modules()), len(self.ns3_modules))
            self.assertFalse(get_test_enabled())
            self.assertEqual(len(get_programs_list()), len(self.ns3_executables))

    def test_08_DryRun(self):
        """!
        Test dry-run (printing commands to be executed instead of running them)
        @return None
        """
        run_ns3("clean")

        for positional_command in ["configure", "build", "clean"]:
            return_code, stdout, stderr = run_ns3("--dry-run %s" % positional_command)
            return_code1, stdout1, stderr1 = run_ns3(
                "%s --dry-run" % positional_command
            )

            self.assertEqual(return_code, return_code1)
            self.assertEqual(stdout, stdout1)
            self.assertEqual(stderr, stderr1)

        run_ns3("clean")

        run_ns3('configure -G "{generator}" -d release --enable-verbose')
        run_ns3("build scratch-simulator")

        return_code0, stdout0, stderr0 = run_ns3("--dry-run run scratch-simulator")
        return_code1, stdout1, stderr1 = run_ns3("run scratch-simulator")
        return_code2, stdout2, stderr2 = run_ns3(
            "--dry-run run scratch-simulator --no-build"
        )
        return_code3, stdout3, stderr3 = run_ns3("run scratch-simulator --no-build")

        self.assertEqual(
            sum([return_code0, return_code1, return_code2, return_code3]), 0
        )
        self.assertEqual([stderr0, stderr1, stderr2, stderr3], [""] * 4)

        scratch_path = None
        for program in get_programs_list():
            if "scratch-simulator" in program and "subdir" not in program:
                scratch_path = program
                break

        self.assertIn(
            cmake_build_target_command(target="scratch_scratch-simulator"), stdout0
        )
        self.assertIn(scratch_path, stdout0)

        self.assertNotIn(
            cmake_build_target_command(target="scratch_scratch-simulator"), stdout1
        )
        self.assertIn("Built target", stdout1)
        self.assertNotIn(scratch_path, stdout1)

        self.assertIn("The following commands would be executed:", stdout2)
        self.assertIn(scratch_path, stdout2)

        self.assertNotIn("Finished executing the following commands:", stdout3)
        self.assertNotIn(scratch_path, stdout3)

    def test_09_PropagationOfReturnCode(self):
        """!
        Test if ns3 is propagating back the return code from the executables called with the run command
        @return None
        """
        return_code, _, _ = run_ns3("clean")
        self.assertEqual(return_code, 0)

        return_code, _, _ = run_ns3(
            'configure -G "{generator}" --enable-examples --enable-tests'
        )
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("build command-line-example test-runner")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3(
            'run "test-runner --test-name=command-line" --no-build'
        )
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3(
            'run "test-runner --test-name=command-line" --no-build',
            env={"NS_COMMANDLINE_INTROSPECTION": ".."},
        )
        self.assertNotEqual(return_code, 0)

        sigsegv_example = os.path.join(ns3_path, "scratch", "sigsegv.cc")
        with open(sigsegv_example, "w", encoding="utf-8") as f:
            f.write("""
                    int main (int argc, char *argv[])
                    {
                      char *s = "hello world"; *s = 'H';
                      return 0;
                    }
                    """)
        return_code, stdout, stderr = run_ns3("run sigsegv")
        if win32:
            self.assertEqual(return_code, 4294967295)
            self.assertIn("sigsegv-default.exe' returned non-zero exit status", stdout)
        else:
            self.assertEqual(return_code, 245)
            self.assertIn("sigsegv-default' died with <Signals.SIGSEGV: 11>", stdout)

        abort_example = os.path.join(ns3_path, "scratch", "abort.cc")
        with open(abort_example, "w", encoding="utf-8") as f:
            f.write("""
                    #include "ns3/core-module.h"

                    using namespace ns3;
                    int main (int argc, char *argv[])
                    {
                      NS_ABORT_IF(true);
                      return 0;
                    }
                    """)
        return_code, stdout, stderr = run_ns3("run abort")
        if win32:
            self.assertEqual(return_code, 3)
            self.assertIn("abort-default.exe' returned non-zero exit status", stdout)
        else:
            self.assertEqual(return_code, 250)
            self.assertIn("abort-default' died with <Signals.SIGABRT: 6>", stdout)

        os.remove(sigsegv_example)
        os.remove(abort_example)

    def test_10_CheckConfig(self):
        """!
        Test passing 'show config' argument to ns3 to get the configuration table
        @return None
        """
        return_code, stdout, stderr = run_ns3("show config")
        self.assertEqual(return_code, 0)
        self.assertIn("Summary of ns-3 settings", stdout)

    def test_11_CheckProfile(self):
        """!
        Test passing 'show profile' argument to ns3 to get the build profile
        @return None
        """
        return_code, stdout, stderr = run_ns3("show profile")
        self.assertEqual(return_code, 0)
        self.assertIn("Build profile: release", stdout)

    def test_12_CheckVersion(self):
        """!
        Test passing 'show version' argument to ns3 to get the build version
        @return None
        """
        if shutil.which("git") is None:
            self.skipTest("git is not available")

        return_code, _, _ = run_ns3('configure -G "{generator}" --enable-build-version')
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("show version")
        self.assertEqual(return_code, 0)
        self.assertIn("ns-3 version:", stdout)

    def test_13_Scratches(self):
        """!
        Test if CMake target names for scratches and ns3 shortcuts
        are working correctly
        @return None
        """

        test_files = [
            "scratch/main.cc",
            "scratch/empty.cc",
            "scratch/subdir1/main.cc",
            "scratch/subdir2/main.cc",
            "scratch/main.test.dots.in.name.cc",
        ]
        backup_files = ["scratch/.main.cc"]

        for path in test_files + backup_files:
            filepath = os.path.join(ns3_path, path)
            os.makedirs(os.path.dirname(filepath), exist_ok=True)
            with open(filepath, "w", encoding="utf-8") as f:
                if "main" in path:
                    f.write("int main (int argc, char *argv[]){}")
                else:
                    f.write("")

        return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
        self.assertEqual(return_code, 0)

        for path in test_files + backup_files:
            path = path.replace(".cc", "")
            return_code1, stdout1, stderr1 = run_program(
                "cmake",
                "--build . --target %s -j %d" % (path.replace("/", "_"), num_threads),
                cwd=os.path.join(ns3_path, "cmake-cache"),
            )
            return_code2, stdout2, stderr2 = run_ns3("build %s" % path)
            if "main" in path and ".main" not in path:
                self.assertEqual(return_code1, 0)
                self.assertEqual(return_code2, 0)
            else:
                self.assertEqual(return_code1, 2)
                self.assertEqual(return_code2, 1)

        for path in test_files:
            path = path.replace(".cc", "")
            return_code, stdout, stderr = run_ns3("run %s --no-build" % path)
            if "main" in path:
                self.assertEqual(return_code, 0)
            else:
                self.assertEqual(return_code, 1)

        run_ns3("clean")
        with DockerContainerManager(self, "ubuntu:18.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 cmake g++-8 ninja-build")
            try:
                container.execute(
                    "./ns3 configure --enable-modules=core,network,internet -- -DCMAKE_CXX_COMPILER=/usr/bin/g++-8"
                )
            except DockerException as e:
                self.fail()
            for path in test_files:
                path = path.replace(".cc", "")
                try:
                    container.execute(f"./ns3 run {path}")
                except DockerException as e:
                    if "main" in path:
                        self.fail()
        run_ns3("clean")

        for path in test_files + backup_files:
            source_absolute_path = os.path.join(ns3_path, path)
            os.remove(source_absolute_path)
            if "empty" in path or ".main" in path:
                continue
            filename = os.path.basename(path).replace(".cc", "")
            executable_absolute_path = os.path.dirname(
                os.path.join(ns3_path, "build", path)
            )
            executable_name = list(
                filter(lambda x: filename in x, os.listdir(executable_absolute_path))
            )[0]

            os.remove(os.path.join(executable_absolute_path, executable_name))
            if not os.listdir(os.path.dirname(path)):
                os.rmdir(os.path.dirname(source_absolute_path))

        return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
        self.assertEqual(return_code, 0)

    def test_14_MpiCommandTemplate(self):
        """!
        Test if ns3 is inserting additional arguments by MPICH and OpenMPI to run on the CI
        @return None
        """
        if shutil.which("mpiexec") is None or win32:
            self.skipTest("Mpi is not available")

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-examples'
        )
        self.assertEqual(return_code, 0)
        executables = get_programs_list()

        return_code, stdout, stderr = run_ns3("build sample-simulator")
        self.assertEqual(return_code, 0)

        sample_simulator_path = list(
            filter(lambda x: "sample-simulator" in x, executables)
        )[0]

        mpi_command = (
            '--dry-run run sample-simulator --command-template="mpiexec -np 2 %s"'
        )
        non_mpi_command = '--dry-run run sample-simulator --command-template="echo %s"'

        return_code, stdout, stderr = run_ns3(mpi_command)
        self.assertEqual(return_code, 0)
        self.assertIn("mpiexec -np 2 %s" % sample_simulator_path, stdout)

        return_code, stdout, stderr = run_ns3(mpi_command)
        self.assertEqual(return_code, 0)
        if os.getenv("USER", "") == "root":
            if shutil.which("ompi_info"):
                self.assertIn(
                    "mpiexec --allow-run-as-root --oversubscribe -np 2 %s"
                    % sample_simulator_path,
                    stdout,
                )
            else:
                self.assertIn(
                    "mpiexec --allow-run-as-root -np 2 %s" % sample_simulator_path,
                    stdout,
                )
        else:
            self.assertIn("mpiexec -np 2 %s" % sample_simulator_path, stdout)

        return_code, stdout, stderr = run_ns3(non_mpi_command)
        self.assertEqual(return_code, 0)
        self.assertIn("echo %s" % sample_simulator_path, stdout)

        return_code, stdout, stderr = run_ns3(non_mpi_command)
        self.assertEqual(return_code, 0)
        self.assertIn("echo %s" % sample_simulator_path, stdout)

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --disable-examples'
        )
        self.assertEqual(return_code, 0)

    def test_15_InvalidLibrariesToLink(self):
        """!
        Test if CMake and ns3 fail in the expected ways when:
        - examples from modules or general examples fail if they depend on a
        library with a name shorter than 4 characters or are disabled when
        a library is nonexistent
        - a module library passes the configuration but fails to build due to
        a missing library
        @return None
        """
        os.makedirs("contrib/borked", exist_ok=True)
        os.makedirs("contrib/borked/examples", exist_ok=True)

        with open("contrib/borked/examples/CMakeLists.txt", "w", encoding="utf-8") as f:
            f.write("")
        for invalid_or_nonexistent_library in ["", "gsd", "lib", "libfi", "calibre"]:
            with open("contrib/borked/CMakeLists.txt", "w", encoding="utf-8") as f:
                f.write(
                    """
                        build_lib(
                            LIBNAME borked
                            SOURCE_FILES ${PROJECT_SOURCE_DIR}/build-support/empty.cc
                            LIBRARIES_TO_LINK ${libcore} %s
                        )
                        """
                    % invalid_or_nonexistent_library
                )

            return_code, stdout, stderr = run_ns3(
                'configure -G "{generator}" --enable-examples'
            )
            if invalid_or_nonexistent_library in ["", "gsd", "libfi", "calibre"]:
                self.assertEqual(return_code, 0)
            elif invalid_or_nonexistent_library in ["lib"]:
                self.assertEqual(return_code, 1)
                self.assertIn(
                    "Invalid library name: %s" % invalid_or_nonexistent_library, stderr
                )
            else:
                pass

            return_code, stdout, stderr = run_ns3("build borked")
            if invalid_or_nonexistent_library in [""]:
                self.assertEqual(return_code, 0)
            elif invalid_or_nonexistent_library in ["lib"]:
                self.assertEqual(return_code, 2)
                self.assertIn(
                    "Invalid library name: %s" % invalid_or_nonexistent_library, stderr
                )
            elif invalid_or_nonexistent_library in ["gsd", "libfi", "calibre"]:
                self.assertEqual(return_code, 2)
                if "lld" in stdout + stderr:
                    self.assertIn(
                        "unable to find library -l%s" % invalid_or_nonexistent_library,
                        stderr,
                    )
                elif "mold" in stdout + stderr:
                    self.assertIn(
                        "library not found: %s" % invalid_or_nonexistent_library, stderr
                    )
                else:
                    self.assertIn(
                        "cannot find -l%s" % invalid_or_nonexistent_library, stderr
                    )
            else:
                pass

        with open("contrib/borked/CMakeLists.txt", "w", encoding="utf-8") as f:
            f.write("""
                    build_lib(
                        LIBNAME borked
                        SOURCE_FILES ${PROJECT_SOURCE_DIR}/build-support/empty.cc
                        LIBRARIES_TO_LINK ${libcore}
                    )
                    """)
        for invalid_or_nonexistent_library in ["", "gsd", "lib", "libfi", "calibre"]:
            with open(
                "contrib/borked/examples/CMakeLists.txt", "w", encoding="utf-8"
            ) as f:
                f.write(
                    """
                        build_lib_example(
                            NAME borked-example
                            SOURCE_FILES ${PROJECT_SOURCE_DIR}/build-support/empty-main.cc
                            LIBRARIES_TO_LINK ${libborked} %s
                        )
                        """
                    % invalid_or_nonexistent_library
                )

            return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
            if invalid_or_nonexistent_library in ["", "gsd", "libfi", "calibre"]:
                self.assertEqual(return_code, 0)
            elif invalid_or_nonexistent_library in ["lib"]:
                self.assertEqual(return_code, 1)
                self.assertIn(
                    "Invalid library name: %s" % invalid_or_nonexistent_library, stderr
                )
            else:
                pass

            return_code, stdout, stderr = run_ns3("build borked-example")
            if invalid_or_nonexistent_library in [""]:
                self.assertEqual(return_code, 0)
            elif invalid_or_nonexistent_library in ["libf"]:
                self.assertEqual(return_code, 2)
                self.assertIn(
                    "Invalid library name: %s" % invalid_or_nonexistent_library, stderr
                )
            elif invalid_or_nonexistent_library in ["gsd", "libfi", "calibre"]:
                self.assertEqual(return_code, 1)
                self.assertIn("Target to build does not exist: borked-example", stdout)
            else:
                pass

        shutil.rmtree("contrib/borked", ignore_errors=True)

    def test_16_LibrariesContainingLib(self):
        """!
        Test if CMake can properly handle modules containing "lib",
        which is used internally as a prefix for module libraries
        @return None
        """

        os.makedirs("contrib/calibre", exist_ok=True)
        os.makedirs("contrib/calibre/examples", exist_ok=True)

        with open(
            "contrib/calibre/examples/CMakeLists.txt", "w", encoding="utf-8"
        ) as f:
            f.write("")
        with open("contrib/calibre/CMakeLists.txt", "w", encoding="utf-8") as f:
            f.write("""
                build_lib(
                    LIBNAME calibre
                    SOURCE_FILES ${PROJECT_SOURCE_DIR}/build-support/empty.cc
                    LIBRARIES_TO_LINK ${libcore}
                )
                """)

        return_code, stdout, stderr = run_ns3('configure -G "{generator}"')

        self.assertEqual(return_code, 0)

        self.assertIn("calibre", stdout)

        self.assertNotIn("care", stdout)
        self.assertTrue(
            os.path.exists(
                os.path.join(ns3_path, "cmake-cache", "pkgconfig", "ns3-calibre.pc")
            )
        )

        return_code, stdout, stderr = run_ns3("build calibre")
        self.assertEqual(return_code, 0)
        self.assertIn(cmake_build_target_command(target="libcalibre"), stdout)

        shutil.rmtree("contrib/calibre", ignore_errors=True)

    def test_17_CMakePerformanceTracing(self):
        """!
        Test if CMake performance tracing works and produces the
        cmake_performance_trace.log file
        @return None
        """
        return_code, stdout, stderr = run_ns3("configure --trace-performance")
        self.assertEqual(return_code, 0)
        if win32:
            self.assertIn("--profiling-format=google-trace --profiling-output=", stdout)
        else:
            self.assertIn(
                "--profiling-format=google-trace --profiling-output=../cmake_performance_trace.log",
                stdout,
            )
        self.assertTrue(
            os.path.exists(os.path.join(ns3_path, "cmake_performance_trace.log"))
        )

    def test_18_CheckBuildVersionAndVersionCache(self):
        """!
        Check if ENABLE_BUILD_VERSION and version.cache are working
        as expected
        @return None
        """

        with DockerContainerManager(self, "ubuntu:22.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 ninja-build cmake g++")

            container.execute("./ns3 clean")

            version_cache_file = os.path.join(ns3_path, "src/core/model/version.cache")

            if os.path.exists(version_cache_file):
                os.remove(version_cache_file)

            try:
                container.execute("./ns3 configure -G Ninja --enable-build-version")
            except DockerException:
                pass
            self.assertFalse(
                os.path.exists(os.path.join(ns3_path, "cmake-cache", "build.ninja"))
            )

            version_cache_contents = (
                "CLOSEST_TAG = '\"ns-3.0.0\"'\n"
                "VERSION_COMMIT_HASH = '\"0000000000\"'\n"
                "VERSION_DIRTY_FLAG = '0'\n"
                "VERSION_MAJOR = '3'\n"
                "VERSION_MINOR = '0'\n"
                "VERSION_PATCH = '0'\n"
                "VERSION_RELEASE_CANDIDATE = '\"\"'\n"
                "VERSION_TAG = '\"ns-3.0.0\"'\n"
                "VERSION_TAG_DISTANCE = '0'\n"
                "VERSION_BUILD_PROFILE = 'debug'\n"
            )
            with open(version_cache_file, "w", encoding="utf-8") as version:
                version.write(version_cache_contents)

            container.execute("./ns3 clean")
            container.execute("./ns3 configure -G Ninja --enable-build-version")
            container.execute("./ns3 build core")
            self.assertTrue(
                os.path.exists(os.path.join(ns3_path, "cmake-cache", "build.ninja"))
            )

            with open(version_cache_file, "r", encoding="utf-8") as version:
                self.assertEqual(version.read(), version_cache_contents)

            os.rename(
                os.path.join(ns3_path, ".git"), os.path.join(ns3_path, "temp_git")
            )
            try:
                container.execute("apt-get install -y git")
                container.execute("./ns3 clean")
                container.execute("./ns3 configure -G Ninja --enable-build-version")
                container.execute("./ns3 build core")
            except DockerException:
                pass
            os.rename(
                os.path.join(ns3_path, "temp_git"), os.path.join(ns3_path, ".git")
            )
            self.assertTrue(
                os.path.exists(os.path.join(ns3_path, "cmake-cache", "build.ninja"))
            )

            container.execute("./ns3 clean")
            container.execute("./ns3 configure -G Ninja --enable-build-version")
            container.execute("./ns3 build core")
            self.assertTrue(
                os.path.exists(os.path.join(ns3_path, "cmake-cache", "build.ninja"))
            )
            with open(version_cache_file, "r", encoding="utf-8") as version:
                self.assertNotEqual(version.read(), version_cache_contents)

            if os.path.exists(version_cache_file):
                os.remove(version_cache_file)

    def test_19_FilterModuleExamplesAndTests(self):
        """!
        Test filtering in examples and tests from specific modules
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-examples --enable-tests'
        )
        self.config_ok(return_code, stdout)

        modules_before_filtering = get_enabled_modules()
        programs_before_filtering = get_programs_list()

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --filter-module-examples-and-tests='core;network'"
        )
        self.config_ok(return_code, stdout)

        modules_after_filtering = get_enabled_modules()
        programs_after_filtering = get_programs_list()

        self.assertEqual(len(modules_after_filtering), len(modules_before_filtering))
        self.assertLess(len(programs_after_filtering), len(programs_before_filtering))

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --filter-module-examples-and-tests='core'"
        )
        self.config_ok(return_code, stdout)

        self.assertEqual(len(get_enabled_modules()), len(modules_after_filtering))
        self.assertLess(len(get_programs_list()), len(programs_after_filtering))

        return_code, stdout, stderr = run_ns3(
            "configure -G \"{generator}\" --disable-examples --disable-tests --filter-module-examples-and-tests=''"
        )
        self.config_ok(return_code, stdout)

        self.assertEqual(len(get_enabled_modules()), len(self.ns3_modules))
        self.assertEqual(len(get_programs_list()), len(self.ns3_executables))

    def test_20_CheckFastLinkers(self):
        """!
        Check if fast linkers LLD and Mold are correctly found and configured
        @return None
        """

        run_ns3("clean")
        with DockerContainerManager(self, "gcc:12.1") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 ninja-build cmake g++ lld")

            container.execute("./ns3 configure -G Ninja")

            self.assertTrue(
                os.path.exists(os.path.join(ns3_path, "cmake-cache", "build.ninja"))
            )
            with open(
                os.path.join(ns3_path, "cmake-cache", "build.ninja"),
                "r",
                encoding="utf-8",
            ) as f:
                self.assertIn("-fuse-ld=lld", f.read())

            try:
                container.execute("./ns3 build core")
            except DockerException:
                self.assertTrue(False, "Build with lld failed")

            if not os.path.exists("./mold-1.4.2-x86_64-linux.tar.gz"):
                container.execute(
                    "wget https://github.com/rui314/mold/releases/download/v1.4.2/mold-1.4.2-x86_64-linux.tar.gz"
                )
            container.execute(
                "tar xzfC mold-1.4.2-x86_64-linux.tar.gz /usr/local --strip-components=1"
            )

            run_ns3("clean")
            container.execute("./ns3 configure -G Ninja")

            self.assertTrue(
                os.path.exists(os.path.join(ns3_path, "cmake-cache", "build.ninja"))
            )
            with open(
                os.path.join(ns3_path, "cmake-cache", "build.ninja"),
                "r",
                encoding="utf-8",
            ) as f:
                self.assertIn("-fuse-ld=mold", f.read())

            try:
                container.execute("./ns3 build core")
            except DockerException:
                self.assertTrue(False, "Build with mold failed")

            os.remove("./mold-1.4.2-x86_64-linux.tar.gz")

            container.execute("./ns3 configure -G Ninja -- -DNS3_FAST_LINKERS=OFF")

            self.assertTrue(
                os.path.exists(os.path.join(ns3_path, "cmake-cache", "build.ninja"))
            )
            with open(
                os.path.join(ns3_path, "cmake-cache", "build.ninja"),
                "r",
                encoding="utf-8",
            ) as f:
                self.assertNotIn("-fuse-ld=mold", f.read())

    def test_21_ClangTimeTrace(self):
        """!
        Check if NS3_CLANG_TIMETRACE feature is working
        Clang's -ftime-trace plus ClangAnalyzer report
        @return None
        """

        run_ns3("clean")
        with DockerContainerManager(self, "ubuntu:20.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 ninja-build cmake clang-10")

            try:
                container.execute(
                    "./ns3 configure -G Ninja --enable-modules=core --enable-examples --enable-tests -- -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 -DNS3_CLANG_TIMETRACE=ON"
                )
            except DockerException as e:
                self.assertIn(
                    "could not find git for clone of ClangBuildAnalyzer", e.stderr
                )

            container.execute("apt-get install -y git")

            try:
                container.execute(
                    "./ns3 configure -G Ninja --enable-modules=core --enable-examples --enable-tests -- -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 -DNS3_CLANG_TIMETRACE=ON"
                )
            except DockerException as e:
                self.assertIn(
                    "could not find git for clone of ClangBuildAnalyzer", e.stderr
                )

            time_trace_report_path = os.path.join(
                ns3_path, "ClangBuildAnalyzerReport.txt"
            )
            if os.path.exists(time_trace_report_path):
                os.remove(time_trace_report_path)

            try:
                container.execute("./ns3 build timeTraceReport")
            except DockerException as e:
                self.assertTrue(
                    False, "Failed to build the ClangAnalyzer's time trace report"
                )

            self.assertTrue(os.path.exists(time_trace_report_path))

            run_ns3("clean")
            container.execute("apt-get install -y g++")
            container.execute("apt-get remove -y clang-10")

            try:
                container.execute(
                    "./ns3 configure -G Ninja --enable-modules=core --enable-examples --enable-tests -- -DNS3_CLANG_TIMETRACE=ON"
                )
                self.assertTrue(
                    False,
                    "ClangTimeTrace requires Clang, but GCC just passed the checks too",
                )
            except DockerException as e:
                self.assertIn("TimeTrace is a Clang feature", e.stderr)

    def test_22_NinjaTrace(self):
        """!
        Check if NS3_NINJA_TRACE feature is working
        Ninja's .ninja_log conversion to about://tracing
        json format conversion with Ninjatracing
        @return None
        """

        run_ns3("clean")
        with DockerContainerManager(self, "ubuntu:20.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 cmake clang-10")

            try:
                container.execute(
                    "./ns3 configure --enable-modules=core --enable-ninja-tracing -- -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10"
                )
            except DockerException as e:
                self.assertIn("Ninjatracing requires the Ninja generator", e.stderr)

            run_ns3("clean")

            container.execute("apt-get install -y ninja-build")
            try:
                container.execute(
                    "./ns3 configure -G Ninja --enable-modules=core --enable-ninja-tracing -- -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10"
                )
            except DockerException as e:
                self.assertIn("could not find git for clone of NinjaTracing", e.stderr)

            container.execute("apt-get install -y git")
            try:
                container.execute(
                    "./ns3 configure -G Ninja --enable-modules=core --enable-ninja-tracing -- -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10"
                )
            except DockerException as e:
                self.assertTrue(False, "Failed to configure with Ninjatracing")

            ninja_trace_path = os.path.join(ns3_path, "ninja_performance_trace.json")
            if os.path.exists(ninja_trace_path):
                os.remove(ninja_trace_path)

            container.execute("./ns3 build core")

            try:
                container.execute("./ns3 build ninjaTrace")
            except DockerException as e:
                self.assertTrue(
                    False, "Failed to run Ninjatracing's tool to build the trace"
                )

            self.assertTrue(os.path.exists(ninja_trace_path))
            trace_size = os.stat(ninja_trace_path).st_size
            os.remove(ninja_trace_path)

            run_ns3("clean")

            try:
                container.execute(
                    "./ns3 configure -G Ninja --enable-modules=core --enable-ninja-tracing -- -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 -DNS3_CLANG_TIMETRACE=ON"
                )
            except DockerException as e:
                self.assertTrue(
                    False, "Failed to configure Ninjatracing with Clang's TimeTrace"
                )

            container.execute("./ns3 build core")

            try:
                container.execute("./ns3 build ninjaTrace")
            except DockerException as e:
                self.assertTrue(
                    False, "Failed to run Ninjatracing's tool to build the trace"
                )

            self.assertTrue(os.path.exists(ninja_trace_path))
            timetrace_size = os.stat(ninja_trace_path).st_size
            os.remove(ninja_trace_path)

            self.assertGreater(timetrace_size, trace_size)

    def test_23_PrecompiledHeaders(self):
        """!
        Check if precompiled headers are being enabled correctly.
        @return None
        """

        run_ns3("clean")
        with DockerContainerManager(self, "ubuntu:18.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 cmake ccache clang-10")
            try:
                container.execute(
                    "./ns3 configure -- -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10"
                )
            except DockerException as e:
                self.assertIn("does not support precompiled headers", e.stderr)
        run_ns3("clean")

        with DockerContainerManager(self, "ubuntu:20.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 cmake ccache g++")
            try:
                container.execute("./ns3 configure")
            except DockerException as e:
                self.assertIn("incompatible with ccache", e.stderr)
        run_ns3("clean")

        with DockerContainerManager(self, "ubuntu:22.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 cmake ccache g++")
            try:
                container.execute("./ns3 configure")
            except DockerException as e:
                self.assertTrue(False, "Precompiled headers should have been enabled")

    def test_24_CheckTestSettings(self):
        """!
        Check for regressions in test object build.
        @return None
        """
        return_code, stdout, stderr = run_ns3("configure")
        self.assertEqual(return_code, 0)

        test_module_cache = os.path.join(ns3_path, "cmake-cache", "src", "test")
        self.assertFalse(os.path.exists(test_module_cache))

        return_code, stdout, stderr = run_ns3("configure --enable-tests")
        self.assertEqual(return_code, 0)
        self.assertTrue(os.path.exists(test_module_cache))


class NS3BuildBaseTestCase(NS3BaseTestCase):
    """!
    Tests ns3 regarding building the project
    """

    def setUp(self):
        """!
        Reuse cleaning/release configuration from NS3BaseTestCase if flag is cleaned
        @return None
        """
        super().setUp()

        self.ns3_libraries = get_libraries_list()

    def test_01_BuildExistingTargets(self):
        """!
        Try building the core library
        @return None
        """
        return_code, stdout, stderr = run_ns3("build core")
        self.assertEqual(return_code, 0)
        self.assertIn("Built target libcore", stdout)

    def test_02_BuildNonExistingTargets(self):
        """!
        Try building core-test library without tests enabled
        @return None
        """
        return_code, stdout, stderr = run_ns3("build core-test")
        self.assertEqual(return_code, 1)
        self.assertIn("Target to build does not exist: core-test", stdout)

    def test_03_BuildProject(self):
        """!
        Try building the project:
        @return None
        """
        return_code, stdout, stderr = run_ns3("build")
        self.assertEqual(return_code, 0)
        self.assertIn("Built target", stdout)
        for program in get_programs_list():
            self.assertTrue(os.path.exists(program), program)
        self.assertIn(cmake_build_project_command, stdout)

    def test_04_BuildProjectNoTaskLines(self):
        """!
        Try hiding task lines
        @return None
        """
        return_code, stdout, stderr = run_ns3("--quiet build")
        self.assertEqual(return_code, 0)
        self.assertIn(cmake_build_project_command, stdout)

    def test_05_BreakBuild(self):
        """!
        Try removing an essential file to break the build
        @return None
        """
        attribute_cc_path = os.sep.join(
            [ns3_path, "src", "core", "model", "attribute.cc"]
        )
        attribute_cc_bak_path = attribute_cc_path + ".bak"
        shutil.move(attribute_cc_path, attribute_cc_bak_path)

        return_code, stdout, stderr = run_ns3("build")
        self.assertNotEqual(return_code, 0)

        shutil.move(attribute_cc_bak_path, attribute_cc_path)

        return_code, stdout, stderr = run_ns3("build")
        self.assertEqual(return_code, 0)

    def test_06_TestVersionFile(self):
        """!
        Test if changing the version file affects the library names
        @return None
        """
        run_ns3("build")
        self.ns3_libraries = get_libraries_list()

        version_file = os.sep.join([ns3_path, "VERSION"])
        with open(version_file, "w", encoding="utf-8") as f:
            f.write("3-00\n")

        return_code, stdout, stderr = run_ns3('configure -G "{generator}"')
        self.config_ok(return_code, stdout)

        return_code, stdout, stderr = run_ns3("build")
        self.assertEqual(return_code, 0)
        self.assertIn("Built target", stdout)

        new_programs = get_programs_list()

        for program in new_programs:
            self.assertTrue(os.path.exists(program))

        self.assertEqual(len(new_programs), len(self.ns3_executables))

        libraries = get_libraries_list()
        new_libraries = list(set(libraries).difference(set(self.ns3_libraries)))
        self.assertEqual(len(new_libraries), len(self.ns3_libraries))
        for library in new_libraries:
            self.assertNotIn("libns3-dev", library)
            self.assertIn("libns3-00", library)
            self.assertTrue(os.path.exists(library))

        with open(version_file, "w", encoding="utf-8") as f:
            f.write("3-dev\n")

    def test_07_OutputDirectory(self):
        """!
        Try setting a different output directory and if everything is
        in the right place and still working correctly
        @return None
        """

        return_code, stdout, stderr = run_ns3("build")
        self.assertEqual(return_code, 0)

        self.ns3_libraries = get_libraries_list()

        self.ns3_executables = get_programs_list()

        for program in self.ns3_executables:
            os.remove(program)
        for library in self.ns3_libraries:
            os.remove(library)

        absolute_path = os.sep.join([ns3_path, "build", "release"])
        relative_path = os.sep.join(["build", "release"])
        for different_out_dir in [absolute_path, relative_path]:
            return_code, stdout, stderr = run_ns3(
                'configure -G "{generator}" --out=%s' % different_out_dir
            )
            self.config_ok(return_code, stdout)
            self.assertIn(
                "Build directory               : %s"
                % absolute_path.replace(os.sep, "/"),
                stdout,
            )

            run_ns3("build")

            new_programs = get_programs_list()
            self.assertEqual(len(new_programs), len(self.ns3_executables))
            for program in new_programs:
                self.assertTrue(os.path.exists(program))

            libraries = get_libraries_list(os.sep.join([absolute_path, "lib"]))
            new_libraries = list(set(libraries).difference(set(self.ns3_libraries)))
            self.assertEqual(len(new_libraries), len(self.ns3_libraries))
            for library in new_libraries:
                self.assertTrue(os.path.exists(library))

            shutil.rmtree(absolute_path)

        return_code, stdout, stderr = run_ns3("configure -G \"{generator}\" --out=''")
        self.config_ok(return_code, stdout)
        self.assertIn(
            "Build directory               : %s" % usual_outdir.replace(os.sep, "/"),
            stdout,
        )

        run_ns3("build")

        new_programs = get_programs_list()
        self.assertEqual(len(new_programs), len(self.ns3_executables))
        for program in new_programs:
            self.assertTrue(os.path.exists(program))

        libraries = get_libraries_list()
        self.assertEqual(len(libraries), len(self.ns3_libraries))
        for library in libraries:
            self.assertTrue(os.path.exists(library))

    def test_08_InstallationAndUninstallation(self):
        """!
        Tries setting a ns3 version, then installing it.
        After that, tries searching for ns-3 with CMake's find_package(ns3).
        Finally, tries using core library in a 3rd-party project
        @return None
        """
        libraries = get_libraries_list()
        for library in libraries:
            os.remove(library)

        version_file = os.sep.join([ns3_path, "VERSION"])
        with open(version_file, "w", encoding="utf-8") as f:
            f.write("3-01\n")

        install_prefix = os.sep.join([ns3_path, "build", "install"])
        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --prefix=%s' % install_prefix
        )
        self.config_ok(return_code, stdout)

        run_ns3("build")
        libraries = get_libraries_list()
        headers = get_headers_list()

        run_ns3("install")

        lib64 = os.path.exists(os.sep.join([install_prefix, "lib64"]))
        installed_libdir = os.sep.join([install_prefix, ("lib64" if lib64 else "lib")])

        installed_libraries = get_libraries_list(installed_libdir)
        installed_libraries_list = ";".join(installed_libraries)
        for library in libraries:
            library_name = os.path.basename(library)
            self.assertIn(library_name, installed_libraries_list)

        installed_headers = get_headers_list(install_prefix)
        missing_headers = list(
            set([os.path.basename(x) for x in headers])
            - (set([os.path.basename(x) for x in installed_headers]))
        )
        self.assertEqual(len(missing_headers), 0)

        test_main_file = os.sep.join([install_prefix, "main.cpp"])
        with open(test_main_file, "w", encoding="utf-8") as f:
            f.write("""
            #include <ns3/core-module.h>
            using namespace ns3;
            int main ()
            {
                Simulator::Stop (Seconds (1.0));
                Simulator::Run ();
                Simulator::Destroy ();
                return 0;
            }
            """)

        for version in ["", "3.01", "3.00"]:
            ns3_import_methods = []

            cmake_find_package_import = """
                                  list(APPEND CMAKE_PREFIX_PATH ./{lib}/cmake/ns3)
                                  find_package(ns3 {version} COMPONENTS libcore)
                                  target_link_libraries(test PRIVATE ns3::libcore)
                                  """.format(
                lib=("lib64" if lib64 else "lib"), version=version
            )
            ns3_import_methods.append(cmake_find_package_import)

            pkgconfig_import = """
                               list(APPEND CMAKE_PREFIX_PATH ./)
                               include(FindPkgConfig)
                               pkg_check_modules(ns3 REQUIRED IMPORTED_TARGET ns3-core{version})
                               target_link_libraries(test PUBLIC PkgConfig::ns3)
                               """.format(
                lib=("lib64" if lib64 else "lib"),
                version="=" + version if version else "",
            )
            if shutil.which("pkg-config"):
                ns3_import_methods.append(pkgconfig_import)

            for import_method in ns3_import_methods:
                test_cmake_project = (
                    """
                                     cmake_minimum_required(VERSION 3.10..3.10)
                                     project(ns3_consumer CXX)
                                     set(CMAKE_CXX_STANDARD 17)
                                     set(CMAKE_CXX_STANDARD_REQUIRED ON)
                                     add_executable(test main.cpp)
                                     """
                    + import_method
                )

                test_cmake_project_file = os.sep.join(
                    [install_prefix, "CMakeLists.txt"]
                )
                with open(test_cmake_project_file, "w", encoding="utf-8") as f:
                    f.write(test_cmake_project)

                cmake = shutil.which("cmake")
                return_code, stdout, stderr = run_program(
                    cmake,
                    '-DCMAKE_BUILD_TYPE=debug -G"{generator}" .'.format(
                        generator=platform_makefiles
                    ),
                    cwd=install_prefix,
                )

                if version == "3.00":
                    self.assertEqual(return_code, 1)
                    if import_method == cmake_find_package_import:
                        self.assertIn(
                            'Could not find a configuration file for package "ns3" that is compatible',
                            stderr.replace("\n", ""),
                        )
                    elif import_method == pkgconfig_import:
                        self.assertIn(
                            "A required package was not found", stderr.replace("\n", "")
                        )
                    else:
                        raise Exception("Unknown import type")
                else:
                    self.assertEqual(return_code, 0)
                    self.assertIn("Build files", stdout)

                return_code, stdout, stderr = run_program(
                    "cmake", "--build .", cwd=install_prefix
                )

                if version == "3.00":
                    self.assertEqual(return_code, 2)
                    self.assertGreater(len(stderr), 0)
                else:
                    self.assertEqual(return_code, 0)
                    self.assertIn("Built target", stdout)

                    if win32:
                        test_program = os.path.join(install_prefix, "test.exe")
                        env_sep = ";" if ";" in os.environ["PATH"] else ":"
                        env = {
                            "PATH": env_sep.join(
                                [
                                    os.environ["PATH"],
                                    os.path.join(install_prefix, "lib"),
                                ]
                            )
                        }
                    else:
                        test_program = "./test"
                        env = None
                    return_code, stdout, stderr = run_program(
                        test_program, "", cwd=install_prefix, env=env
                    )
                    self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("uninstall")
        self.assertIn("Built target uninstall", stdout)

        os.remove(version_file)
        with open(version_file, "w", encoding="utf-8") as f:
            f.write("3-dev\n")

    def test_09_Scratches(self):
        """!
        Tries to build scratch-simulator and subdir/scratch-simulator-subdir
        @return None
        """
        targets = {
            "scratch/scratch-simulator": "scratch-simulator",
            "scratch/scratch-simulator.cc": "scratch-simulator",
            "scratch-simulator": "scratch-simulator",
            "scratch/subdir/scratch-subdir": "subdir_scratch-subdir",
            "subdir/scratch-subdir": "subdir_scratch-subdir",
            "scratch-subdir": "subdir_scratch-subdir",
        }
        for target_to_run, target_cmake in targets.items():
            build_line = "target scratch_%s" % target_cmake
            return_code, stdout, stderr = run_ns3("build %s" % target_to_run)
            self.assertEqual(return_code, 0)
            self.assertIn(build_line, stdout)

            return_code, stdout, stderr = run_ns3("run %s --verbose" % target_to_run)
            self.assertEqual(return_code, 0)
            self.assertIn(build_line, stdout)
            stdout = stdout.replace("scratch_%s" % target_cmake, "")
            self.assertIn(target_to_run.split("/")[-1].replace(".cc", ""), stdout)

    def test_10_AmbiguityCheck(self):
        """!
        Test if ns3 can alert correctly in case a shortcut collision happens
        @return None
        """

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-examples'
        )
        self.assertEqual(return_code, 0)

        shutil.copy("./examples/tutorial/second.cc", "./scratch/second.cc")

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-examples'
        )
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("build second")
        self.assertEqual(return_code, 1)
        self.assertIn(
            'Build target "second" is ambiguous. Try one of these: "scratch/second", "examples/tutorial/second"',
            stdout.replace(os.sep, "/"),
        )

        return_code, stdout, stderr = run_ns3("build scratch/second")
        self.assertEqual(return_code, 0)
        self.assertIn(cmake_build_target_command(target="scratch_second"), stdout)

        return_code, stdout, stderr = run_ns3("build tutorial/second")
        self.assertEqual(return_code, 0)
        self.assertIn(cmake_build_target_command(target="second"), stdout)

        return_code, stdout, stderr = run_ns3("run second")
        self.assertEqual(return_code, 1)
        self.assertIn(
            'Run target "second" is ambiguous. Try one of these: "scratch/second", "examples/tutorial/second"',
            stdout.replace(os.sep, "/"),
        )

        return_code, stdout, stderr = run_ns3("run scratch/second")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("run tutorial/second")
        self.assertEqual(return_code, 0)

        os.remove("./scratch/second.cc")

    def test_11_StaticBuilds(self):
        """!
        Test if we can build a static ns-3 library and link it to static programs
        @return None
        """

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-examples --disable-gtk --enable-static'
        )

        if win32:
            self.assertEqual(return_code, 1)
            self.assertIn("Static builds are unsupported on Windows", stderr)
        else:
            self.assertEqual(return_code, 0)

            return_code, stdout, stderr = run_ns3("build sample-simulator")
            self.assertEqual(return_code, 0)
            self.assertIn("Built target", stdout)

    def test_12_CppyyBindings(self):
        """!
        Test if we can use python bindings
        @return None
        """
        try:
            import cppyy
        except ModuleNotFoundError:
            self.skipTest("Cppyy was not found")

        return_code, stdout, stderr = run_ns3(
            'configure -G "{generator}" --enable-examples --enable-python-bindings'
        )

        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_program("test.py", "", python=True)
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_program(
            "test.py", "-p mixed-wired-wireless", python=True
        )
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_program(
            "test.py", "-p ./examples/wireless/mixed-wired-wireless", python=True
        )
        self.assertEqual(return_code, 0)

    def test_13_FetchOptionalComponents(self):
        """!
        Test if we had regressions with brite, click and openflow modules
        that depend on homonymous libraries
        @return None
        """
        if shutil.which("git") is None:
            self.skipTest("Missing git")

        return_code, stdout, stderr = run_ns3(
            "configure -- -DNS3_FETCH_OPTIONAL_COMPONENTS=ON"
        )
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("build brite click openflow")
        self.assertEqual(return_code, 0)

    def test_14_LinkContribModuleToSrcModule(self):
        """!
        Test if we can link contrib modules to src modules
        @return None
        """
        if shutil.which("git") is None:
            self.skipTest("Missing git")

        destination_contrib = os.path.join(ns3_path, "contrib/test-contrib-dependency")
        destination_src = os.path.join(ns3_path, "src/test-src-dependant-on-contrib")
        if os.path.exists(destination_contrib):
            shutil.rmtree(destination_contrib)
        if os.path.exists(destination_src):
            shutil.rmtree(destination_src)

        shutil.copytree(
            os.path.join(ns3_path, "build-support/test-files/test-contrib-dependency"),
            destination_contrib,
        )
        shutil.copytree(
            os.path.join(
                ns3_path, "build-support/test-files/test-src-dependant-on-contrib"
            ),
            destination_src,
        )

        return_code, stdout, stderr = run_ns3("configure --enable-examples")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3("run source-example")
        self.assertEqual(return_code, 0)

        shutil.rmtree(destination_contrib)
        shutil.rmtree(destination_src)


class NS3ExpectedUseTestCase(NS3BaseTestCase):
    """!
    Tests ns3 usage in more realistic scenarios
    """

    def setUp(self):
        """!
        Reuse cleaning/release configuration from NS3BaseTestCase if flag is cleaned
        Here examples, tests and documentation are also enabled.
        @return None
        """

        super().setUp()

        return_code, stdout, stderr = run_ns3(
            'configure -d release -G "{generator}" --enable-examples --enable-tests'
        )
        self.config_ok(return_code, stdout)

        self.assertTrue(os.path.exists(ns3_lock_filename))

        self.ns3_executables = get_programs_list()

        self.assertTrue(os.path.exists(ns3_lock_filename))

        self.ns3_modules = get_enabled_modules()

    def test_01_BuildProject(self):
        """!
        Try to build the project
        @return None
        """
        return_code, stdout, stderr = run_ns3("build")
        self.assertEqual(return_code, 0)
        self.assertIn("Built target", stdout)
        for program in get_programs_list():
            self.assertTrue(os.path.exists(program))
        libraries = get_libraries_list()
        for module in get_enabled_modules():
            self.assertIn(module.replace("ns3-", ""), ";".join(libraries))
        self.assertIn(cmake_build_project_command, stdout)

    def test_02_BuildAndRunExistingExecutableTarget(self):
        """!
        Try to build and run test-runner
        @return None
        """
        return_code, stdout, stderr = run_ns3('run "test-runner --list" --verbose')
        self.assertEqual(return_code, 0)
        self.assertIn("Built target test-runner", stdout)
        self.assertIn(cmake_build_target_command(target="test-runner"), stdout)

    def test_03_BuildAndRunExistingLibraryTarget(self):
        """!
        Try to build and run a library
        @return None
        """
        return_code, stdout, stderr = run_ns3("run core")
        self.assertEqual(return_code, 1)
        self.assertIn("Couldn't find the specified program: core", stderr)

    def test_04_BuildAndRunNonExistingTarget(self):
        """!
        Try to build and run an unknown target
        @return None
        """
        return_code, stdout, stderr = run_ns3("run nonsense")
        self.assertEqual(return_code, 1)
        self.assertIn("Couldn't find the specified program: nonsense", stderr)

    def test_05_RunNoBuildExistingExecutableTarget(self):
        """!
        Try to run test-runner without building
        @return None
        """
        return_code, stdout, stderr = run_ns3("build test-runner")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3(
            'run "test-runner --list" --no-build --verbose'
        )
        self.assertEqual(return_code, 0)
        self.assertNotIn("Built target test-runner", stdout)
        self.assertNotIn(cmake_build_target_command(target="test-runner"), stdout)

    def test_06_RunNoBuildExistingLibraryTarget(self):
        """!
        Test ns3 fails to run a library
        @return None
        """
        return_code, stdout, stderr = run_ns3("run core --no-build")
        self.assertEqual(return_code, 1)
        self.assertIn("Couldn't find the specified program: core", stderr)

    def test_07_RunNoBuildNonExistingExecutableTarget(self):
        """!
        Test ns3 fails to run an unknown program
        @return None
        """
        return_code, stdout, stderr = run_ns3("run nonsense --no-build")
        self.assertEqual(return_code, 1)
        self.assertIn("Couldn't find the specified program: nonsense", stderr)

    def test_08_RunNoBuildGdb(self):
        """!
        Test if scratch simulator is executed through gdb and lldb
        @return None
        """
        if shutil.which("gdb") is None:
            self.skipTest("Missing gdb")

        return_code, stdout, stderr = run_ns3("build scratch-simulator")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3(
            "run scratch-simulator --gdb --verbose --no-build", env={"gdb_eval": "1"}
        )
        self.assertEqual(return_code, 0)
        self.assertIn("scratch-simulator", stdout)
        if win32:
            self.assertIn("GNU gdb", stdout)
        else:
            self.assertIn("No debugging symbols found", stdout)

    def test_09_RunNoBuildValgrind(self):
        """!
        Test if scratch simulator is executed through valgrind
        @return None
        """
        if shutil.which("valgrind") is None:
            self.skipTest("Missing valgrind")

        return_code, stdout, stderr = run_ns3("build scratch-simulator")
        self.assertEqual(return_code, 0)

        return_code, stdout, stderr = run_ns3(
            "run scratch-simulator --valgrind --verbose --no-build"
        )
        self.assertEqual(return_code, 0)
        self.assertIn("scratch-simulator", stderr)
        self.assertIn("Memcheck", stderr)

    def test_10_DoxygenWithBuild(self):
        """!
        Test the doxygen target that does trigger a full build
        @return None
        """
        if shutil.which("doxygen") is None:
            self.skipTest("Missing doxygen")

        if shutil.which("bash") is None:
            self.skipTest("Missing bash")

        doc_folder = os.path.abspath(os.sep.join([".", "doc"]))

        doxygen_files = ["introspected-command-line.h", "introspected-doxygen.h"]
        for filename in doxygen_files:
            file_path = os.sep.join([doc_folder, filename])
            if os.path.exists(file_path):
                os.remove(file_path)

        return_code, stdout, stderr = run_ns3("docs doxygen")
        self.assertEqual(return_code, 0)
        self.assertIn(cmake_build_target_command(target="doxygen"), stdout)
        self.assertIn("Built target doxygen", stdout)

    def test_11_DoxygenWithoutBuild(self):
        """!
        Test the doxygen target that doesn't trigger a full build
        @return None
        """
        if shutil.which("doxygen") is None:
            self.skipTest("Missing doxygen")

        return_code, stdout, stderr = run_ns3("docs doxygen-no-build")
        self.assertEqual(return_code, 0)
        self.assertIn(cmake_build_target_command(target="doxygen-no-build"), stdout)
        self.assertIn("Built target doxygen-no-build", stdout)

    def test_12_SphinxDocumentation(self):
        """!
        Test every individual target for Sphinx-based documentation
        @return None
        """
        if shutil.which("sphinx-build") is None:
            self.skipTest("Missing sphinx")

        doc_folder = os.path.abspath(os.sep.join([".", "doc"]))

        for target in ["installation", "contributing", "manual", "models", "tutorial"]:
            doc_build_folder = os.sep.join([doc_folder, target, "build"])
            doc_temp_folder = os.sep.join([doc_folder, target, "source-temp"])
            if os.path.exists(doc_build_folder):
                shutil.rmtree(doc_build_folder)
            if os.path.exists(doc_temp_folder):
                shutil.rmtree(doc_temp_folder)

            return_code, stdout, stderr = run_ns3("docs %s" % target)
            self.assertEqual(return_code, 0, target)
            self.assertIn(
                cmake_build_target_command(target="sphinx_%s" % target), stdout
            )
            self.assertIn("Built target sphinx_%s" % target, stdout)

            doc_build_folder = os.sep.join([doc_folder, target, "build"])
            self.assertTrue(os.path.exists(doc_build_folder))

            for build_type in ["latex", "html", "singlehtml"]:
                self.assertTrue(
                    os.path.exists(os.sep.join([doc_build_folder, build_type]))
                )

    def test_13_Documentation(self):
        """!
        Test the documentation target that builds
        both doxygen and sphinx based documentation
        @return None
        """
        if shutil.which("doxygen") is None:
            self.skipTest("Missing doxygen")
        if shutil.which("sphinx-build") is None:
            self.skipTest("Missing sphinx")

        doc_folder = os.path.abspath(os.sep.join([".", "doc"]))

        for target in ["manual", "models", "tutorial"]:
            doc_build_folder = os.sep.join([doc_folder, target, "build"])
            if os.path.exists(doc_build_folder):
                shutil.rmtree(doc_build_folder)

        return_code, stdout, stderr = run_ns3("docs all")
        self.assertEqual(return_code, 0)
        self.assertIn(cmake_build_target_command(target="sphinx"), stdout)
        self.assertIn("Built target sphinx", stdout)
        self.assertIn(cmake_build_target_command(target="doxygen"), stdout)
        self.assertIn("Built target doxygen", stdout)

    def test_14_EnableSudo(self):
        """!
        Try to set ownership of scratch-simulator from current user to root,
        and change execution permissions
        @return None
        """

        sudo_password = os.getenv("SUDO_PASSWORD", None)

        if sudo_password is None:
            self.skipTest("SUDO_PASSWORD environment variable was not specified")

        enable_sudo = read_lock_entry("ENABLE_SUDO")
        self.assertFalse(enable_sudo is True)

        return_code, stdout, stderr = run_ns3("run scratch-simulator")
        self.assertEqual(return_code, 0)
        self.assertIn("Built target scratch_scratch-simulator", stdout)
        self.assertIn(
            cmake_build_target_command(target="scratch_scratch-simulator"), stdout
        )
        scratch_simulator_path = list(
            filter(
                lambda x: x if "scratch-simulator" in x else None, self.ns3_executables
            )
        )[-1]
        prev_fstat = os.stat(scratch_simulator_path)

        return_code, stdout, stderr = run_ns3(
            "run scratch-simulator --enable-sudo", env={"SUDO_PASSWORD": sudo_password}
        )
        self.assertEqual(return_code, 0)
        self.assertIn("Built target scratch_scratch-simulator", stdout)
        self.assertIn(
            cmake_build_target_command(target="scratch_scratch-simulator"), stdout
        )
        fstat = os.stat(scratch_simulator_path)

        import stat

        likely_fuse_mount = (
            (prev_fstat.st_mode & stat.S_ISUID) == (fstat.st_mode & stat.S_ISUID)
        ) and prev_fstat.st_uid == 0  # noqa

        if win32 or likely_fuse_mount:
            self.skipTest("Windows or likely a FUSE mount")

        self.assertEqual(fstat.st_uid, 0)
        self.assertEqual(fstat.st_mode & stat.S_ISUID, stat.S_ISUID)

        return_code, stdout, stderr = run_ns3("configure --enable-sudo")
        self.assertEqual(return_code, 0)

        enable_sudo = read_lock_entry("ENABLE_SUDO")
        self.assertTrue(enable_sudo)

        for executable in self.ns3_executables:
            if os.path.exists(executable):
                os.remove(executable)

        return_code, stdout, stderr = run_ns3(
            "build", env={"SUDO_PASSWORD": sudo_password}
        )
        self.assertEqual(return_code, 0)

        self.assertIn("chown root", stdout)
        self.assertIn("chmod u+s", stdout)
        for executable in self.ns3_executables:
            self.assertIn(os.path.basename(executable), stdout)

        fstat = os.stat(scratch_simulator_path)
        self.assertEqual(fstat.st_uid, 0)
        self.assertEqual(fstat.st_mode & stat.S_ISUID, stat.S_ISUID)

    def test_15_CommandTemplate(self):
        """!
        Check if command template is working
        @return None
        """

        return_code0, stdout0, stderr0 = run_ns3(
            "run sample-simulator --command-template"
        )
        self.assertEqual(return_code0, 2)
        self.assertIn("argument --command-template: expected one argument", stderr0)

        return_code1, stdout1, stderr1 = run_ns3(
            'run sample-simulator --command-template=" "'
        )
        return_code2, stdout2, stderr2 = run_ns3(
            'run sample-simulator --command-template " "'
        )
        return_code3, stdout3, stderr3 = run_ns3(
            'run sample-simulator --command-template "echo "'
        )
        self.assertEqual((return_code1, return_code2, return_code3), (1, 1, 1))
        for stderr in [stderr1, stderr2, stderr3]:
            self.assertIn(
                "not all arguments converted during string formatting", stderr
            )

        return_code4, stdout4, _ = run_ns3(
            'run sample-simulator --command-template "%s --PrintVersion" --verbose'
        )
        return_code5, stdout5, _ = run_ns3(
            'run sample-simulator --command-template="%s --PrintVersion" --verbose'
        )
        self.assertEqual((return_code4, return_code5), (0, 0))

        self.assertIn("sample-simulator{ext} --PrintVersion".format(ext=ext), stdout4)
        self.assertIn("sample-simulator{ext} --PrintVersion".format(ext=ext), stdout5)

    def test_16_ForwardArgumentsToRunTargets(self):
        """!
        Check if all flavors of different argument passing to
        executable targets are working
        @return None
        """

        return_code0, stdout0, stderr0 = run_ns3(
            'run "sample-simulator --help" --verbose'
        )
        return_code1, stdout1, stderr1 = run_ns3(
            'run sample-simulator --command-template="%s --help" --verbose'
        )
        return_code2, stdout2, stderr2 = run_ns3(
            "run sample-simulator --verbose -- --help"
        )

        self.assertEqual((return_code0, return_code1, return_code2), (0, 0, 0))
        self.assertIn("sample-simulator{ext} --help".format(ext=ext), stdout0)
        self.assertIn("sample-simulator{ext} --help".format(ext=ext), stdout1)
        self.assertIn("sample-simulator{ext} --help".format(ext=ext), stdout2)

        return_code0, stdout0, stderr0 = run_ns3(
            'run "sample-simulator --help" --no-build'
        )
        return_code1, stdout1, stderr1 = run_ns3(
            'run sample-simulator --command-template="%s --help" --no-build'
        )
        return_code2, stdout2, stderr2 = run_ns3(
            "run sample-simulator --no-build -- --help"
        )
        self.assertEqual((return_code0, return_code1, return_code2), (0, 0, 0))
        self.assertEqual(stdout0, stdout1)
        self.assertEqual(stdout1, stdout2)
        self.assertEqual(stderr0, stderr1)
        self.assertEqual(stderr1, stderr2)

        return_code0, stdout0, stderr0 = run_ns3(
            'run "sample-simulator --PrintGlobals" --verbose'
        )
        return_code1, stdout1, stderr1 = run_ns3(
            'run "sample-simulator --PrintGroups" --verbose'
        )
        return_code2, stdout2, stderr2 = run_ns3(
            'run "sample-simulator --PrintTypeIds" --verbose'
        )

        self.assertEqual((return_code0, return_code1, return_code2), (0, 0, 0))
        self.assertIn("sample-simulator{ext} --PrintGlobals".format(ext=ext), stdout0)
        self.assertIn("sample-simulator{ext} --PrintGroups".format(ext=ext), stdout1)
        self.assertIn("sample-simulator{ext} --PrintTypeIds".format(ext=ext), stdout2)

        cmd = 'run "sample-simulator --PrintGlobals" --command-template="%s --PrintGroups" --verbose -- --PrintTypeIds'
        return_code, stdout, stderr = run_ns3(cmd)
        self.assertEqual(return_code, 0)

        self.assertIn(
            "sample-simulator{ext} --PrintGroups --PrintGlobals --PrintTypeIds".format(
                ext=ext
            ),
            stdout,
        )

        cmd0 = 'run sample-simulator --command-template="%s " --PrintTypeIds'
        cmd1 = "run sample-simulator --PrintTypeIds"

        return_code0, stdout0, stderr0 = run_ns3(cmd0)
        return_code1, stdout1, stderr1 = run_ns3(cmd1)
        self.assertEqual((return_code0, return_code1), (1, 1))
        self.assertIn(
            "To forward configuration or runtime options, put them after '--'", stderr0
        )
        self.assertIn(
            "To forward configuration or runtime options, put them after '--'", stderr1
        )

    def test_17_RunNoBuildLldb(self):
        """!
        Test if scratch simulator is executed through lldb
        @return None
        """
        if shutil.which("lldb") is None:
            self.skipTest("Missing lldb")

        return_code, stdout, stderr = run_ns3(
            "run scratch-simulator --lldb --verbose --no-build"
        )
        self.assertEqual(return_code, 0)
        self.assertIn("scratch-simulator", stdout)
        self.assertIn("(lldb) target create", stdout)

    def test_18_CpmAndVcpkgManagers(self):
        """!
        Test if CPM and Vcpkg package managers are working properly
        @return None
        """
        return_code, stdout, stderr = run_ns3("clean")
        self.assertEqual(return_code, 0)

        if os.path.exists("vcpkg"):
            shutil.rmtree("vcpkg")

        destination_src = os.path.join(ns3_path, "src/test-package-managers")
        if os.path.exists(destination_src):
            shutil.rmtree(destination_src)

        shutil.copytree(
            os.path.join(ns3_path, "build-support/test-files/test-package-managers"),
            destination_src,
        )

        with DockerContainerManager(self, "ubuntu:22.04") as container:
            container.execute("apt-get update")
            container.execute("apt-get install -y python3 cmake g++ ninja-build")

            try:
                container.execute("./ns3 configure -- -DTEST_PACKAGE_MANAGER:STRING=ON")
                self.skipTest("Armadillo is already installed")
            except DockerException as e:
                pass

            return_code, stdout, stderr = run_ns3("clean")
            self.assertEqual(return_code, 0)

            container.execute("apt-get install -y git")

            try:
                container.execute(
                    "./ns3 configure -- -DNS3_CPM=ON -DTEST_PACKAGE_MANAGER:STRING=CPM"
                )
            except DockerException as e:
                self.fail()

            try:
                container.execute("./ns3 build test-package-managers")
            except DockerException as e:
                self.fail()

            return_code, stdout, stderr = run_ns3("clean")
            self.assertEqual(return_code, 0)

            container.execute("apt-get install -y zip unzip tar curl")

            container.execute("apt-get install -y pkg-config gfortran")

            try:
                container.execute("./ns3 configure -- -DNS3_VCPKG=ON")
            except DockerException as e:
                self.fail()

            try:
                container.execute(
                    "./ns3 configure -- -DTEST_PACKAGE_MANAGER:STRING=VCPKG"
                )
            except DockerException as e:
                self.fail()

            try:
                container.execute("./ns3 build test-package-managers")
            except DockerException as e:
                self.fail()

        if os.path.exists(destination_src):
            shutil.rmtree(destination_src)


class NS3QualityControlTestCase(unittest.TestCase):
    """!
    ns-3 tests to control the quality of the repository over time,
    by checking the state of URLs listed and more
    """

    def test_01_CheckForDeadLinksInSources(self):
        """!
        Test if all urls in source files are alive
        @return None
        """

        try:
            import django
        except ImportError:
            django = None  # noqa
            self.skipTest("Django URL validators are not available")

        try:
            import requests
            import urllib3

            urllib3.disable_warnings()
        except ImportError:
            requests = None  # noqa
            self.skipTest("Requests library is not available")

        regex = re.compile(r"((http|https)://[^\ \n\)\"\'\}\>\<\]\;\`\\]*)")  # noqa
        skipped_files = []

        whitelisted_urls = {
            "https://gitlab.com/your-user-name/ns-3-dev",
            "https://www.nsnam.org/release/ns-allinone-3.31.rc1.tar.bz2",
            "https://www.nsnam.org/release/ns-allinone-3.X.rcX.tar.bz2",
            "https://www.nsnam.org/releases/ns-3-x",
            "https://www.nsnam.org/releases/ns-allinone-3.(x-1",
            "https://www.nsnam.org/releases/ns-allinone-3.x.tar.bz2",
            "https://cmake.org/cmake/help/latest/manual/cmake-",
            "http://www.ieeeghn.org/wiki/index.php/First-Hand:Digital_Television:_The_",
            "http://www.lysator.liu.se/~alla/dia/",
            "http://www.ieeeghn.org/wiki/index.php/First-Hand:Digital_Television:_The_Digital_Terrestrial_Television_Broadcasting_(DTTB",
            "http://en.wikipedia.org/wiki/Namespace_(computer_science",
            "http://en.wikipedia.org/wiki/Bonobo_(component_model",
            "http://msdn.microsoft.com/en-us/library/aa365247(v=vs.85",
            "http://www.research.att.com/info/kpv/",
            "http://www.research.att.com/~gsf/",
        }

        files_and_urls = set()
        unique_urls = set()
        for topdir in ["bindings", "doc", "examples", "src", "utils"]:
            for root, dirs, files in os.walk(topdir):
                if (
                    "build" in root
                    or "_static" in root
                    or "source-temp" in root
                    or "html" in root
                ):
                    continue
                for file in files:
                    filepath = os.path.join(root, file)

                    if not os.path.isfile(filepath):
                        continue

                    if file.endswith(".svg"):
                        continue

                    try:
                        with open(filepath, "r", encoding="utf-8") as f:
                            matches = regex.findall(f.read())

                            urls = list(
                                map(
                                    lambda x: x[0][:-1] if x[0][-1] in ".," else x[0],
                                    matches,
                                )
                            )
                    except UnicodeDecodeError:
                        skipped_files.append(filepath)
                        continue

                    for url in set(urls) - unique_urls - whitelisted_urls:
                        unique_urls.add(url)
                        files_and_urls.add((filepath, url))

        from django.core.validators import URLValidator  # noqa
        from django.core.exceptions import ValidationError  # noqa

        validate_url = URLValidator()

        headers = {
            "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36"
            # noqa
        }

        def test_file_url(args):
            test_filepath, test_url = args
            dead_link_msg = None

            try:
                validate_url(test_url)
            except ValidationError:
                dead_link_msg = "%s: URL %s, invalid URL" % (test_filepath, test_url)
            except Exception as e:
                self.assertEqual(False, True, msg=e.__str__())

            if dead_link_msg is not None:
                return dead_link_msg
            tries = 3
            while tries > 0:
                try:
                    response = requests.get(
                        test_url, verify=False, headers=headers, timeout=50
                    )

                    if response.status_code in [200, 301]:
                        dead_link_msg = None
                        break

                    if response.status_code in [302, 308, 500, 503]:
                        if response.reason.lower() in [
                            "found",
                            "moved temporarily",
                            "permanent redirect",
                            "ok",
                            "service temporarily unavailable",
                        ]:
                            dead_link_msg = None
                            break
                    dead_link_msg = "%s: URL %s: returned code %d" % (
                        test_filepath,
                        test_url,
                        response.status_code,
                    )
                except requests.exceptions.InvalidURL:
                    dead_link_msg = "%s: URL %s: invalid URL" % (
                        test_filepath,
                        test_url,
                    )
                except requests.exceptions.SSLError:
                    dead_link_msg = "%s: URL %s: SSL error" % (test_filepath, test_url)
                except requests.exceptions.TooManyRedirects:
                    dead_link_msg = "%s: URL %s: too many redirects" % (
                        test_filepath,
                        test_url,
                    )
                except Exception as e:
                    try:
                        error_msg = e.args[0].reason.__str__()
                    except AttributeError:
                        error_msg = e.args[0]
                    dead_link_msg = "%s: URL %s: failed with exception: %s" % (
                        test_filepath,
                        test_url,
                        error_msg,
                    )
                tries -= 1
            return dead_link_msg

        from concurrent.futures import ThreadPoolExecutor

        with ThreadPoolExecutor(max_workers=100) as executor:
            dead_links = list(executor.map(test_file_url, list(files_and_urls)))

        dead_links = list(sorted(filter(lambda x: x is not None, dead_links)))
        self.assertEqual(
            len(dead_links), 0, msg="\n".join(["Dead links found:", *dead_links])
        )

    def test_02_MemoryCheckWithSanitizers(self):
        """!
        Test if all tests can be executed without hitting major memory bugs
        @return None
        """
        return_code, stdout, stderr = run_ns3(
            "configure --enable-tests --enable-examples --enable-sanitizers -d optimized"
        )
        self.assertEqual(return_code, 0)

        test_return_code, stdout, stderr = run_program("test.py", "", python=True)
        self.assertEqual(test_return_code, 0)

    def test_03_CheckImageBrightness(self):
        """!
        Check if images in the docs are above a brightness threshold.
        This should prevent screenshots with dark UI themes.
        @return None
        """
        if shutil.which("convert") is None:
            self.skipTest("Imagemagick was not found")

        from pathlib import Path

        image_extensions = ["png", "jpg"]
        images = []
        for extension in image_extensions:
            images += list(
                Path("./doc").glob("**/figures/*.{ext}".format(ext=extension))
            )
            images += list(
                Path("./doc").glob("**/figures/**/*.{ext}".format(ext=extension))
            )

        imagemagick_get_image_brightness = 'convert {image} -colorspace HSI -channel b -separate +channel -scale 1x1 -format "%[fx:100*u]" info:'

        brightness_threshold = 50
        for image in images:
            brightness = subprocess.check_output(
                imagemagick_get_image_brightness.format(image=image).split()
            )
            brightness = float(brightness.decode().strip("'\""))
            self.assertGreater(
                brightness,
                brightness_threshold,
                "Image darker than threshold (%d < %d): %s"
                % (brightness, brightness_threshold, image),
            )


def main():
    """!
    Main function
    @return None
    """

    test_completeness = {
        "style": [
            NS3UnusedSourcesTestCase,
            NS3StyleTestCase,
        ],
        "build": [
            NS3CommonSettingsTestCase,
            NS3ConfigureBuildProfileTestCase,
            NS3ConfigureTestCase,
            NS3BuildBaseTestCase,
            NS3ExpectedUseTestCase,
        ],
        "complete": [
            NS3UnusedSourcesTestCase,
            NS3StyleTestCase,
            NS3CommonSettingsTestCase,
            NS3ConfigureBuildProfileTestCase,
            NS3ConfigureTestCase,
            NS3BuildBaseTestCase,
            NS3ExpectedUseTestCase,
            NS3QualityControlTestCase,
        ],
        "extras": [
            NS3DependenciesTestCase,
        ],
    }

    import argparse

    parser = argparse.ArgumentParser("Test suite for the ns-3 buildsystem")
    parser.add_argument(
        "-c", "--completeness", choices=test_completeness.keys(), default="complete"
    )
    parser.add_argument("-tn", "--test-name", action="store", default=None, type=str)
    parser.add_argument(
        "-rtn", "--resume-from-test-name", action="store", default=None, type=str
    )
    parser.add_argument("-q", "--quiet", action="store_true", default=False)
    args = parser.parse_args(sys.argv[1:])

    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    for testCase in test_completeness[args.completeness]:
        suite.addTests(loader.loadTestsFromTestCase(testCase))

    if args.test_name:
        tests = dict(map(lambda x: (x._testMethodName, x), suite._tests))

        tests_to_run = set(
            map(lambda x: x if args.test_name in x else None, tests.keys())
        )
        tests_to_remove = set(tests) - set(tests_to_run)
        for test_to_remove in tests_to_remove:
            suite._tests.remove(tests[test_to_remove])

    if args.resume_from_test_name:
        tests = dict(map(lambda x: (x._testMethodName, x), suite._tests))
        keys = list(tests.keys())

        while args.resume_from_test_name not in keys[0] and len(tests) > 0:
            suite._tests.remove(tests[keys[0]])
            keys.pop(0)

    ns3rc_script_bak = ns3rc_script + ".bak"
    if os.path.exists(ns3rc_script) and not os.path.exists(ns3rc_script_bak):
        shutil.move(ns3rc_script, ns3rc_script_bak)

    runner = unittest.TextTestRunner(failfast=True, verbosity=1 if args.quiet else 2)
    runner.run(suite)

    if os.path.exists(ns3rc_script_bak):
        shutil.move(ns3rc_script_bak, ns3rc_script)


if __name__ == "__main__":
    main()
