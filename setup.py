import cmake_build_extension
import setuptools

setuptools.setup(
    cmdclass=dict(build_ext=cmake_build_extension.BuildExtension),
    ext_modules=[
        cmake_build_extension.CMakeExtension(
            name="BuildAndInstall",
            install_prefix="ns3",
            cmake_configure_options=[
                "-DCMAKE_BUILD_TYPE:STRING=release",
                "-DNS3_ASSERT:BOOL=ON",
                "-DNS3_LOG:BOOL=ON",
                "-DNS3_WARNINGS_AS_ERRORS:BOOL=OFF",
                "-DNS3_FETCH_OPTIONAL_COMPONENTS:BOOL=ON",
                "-DNS3_PIP_PACKAGING:BOOL=ON",
                "-DNS3_USE_LIB64:BOOL=ON",
            ],
        ),
    ],
)
