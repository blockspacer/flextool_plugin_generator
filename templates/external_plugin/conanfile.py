from conans import ConanFile, CMake, tools, python_requires
import traceback
import os
import shutil
from conans.tools import OSInfo

# if you using python less than 3 use from distutils import strtobool
from distutils.util import strtobool

basis_plugin_helper = python_requires("basis_plugin_helper/[~=0.0]@conan/stable")

class external_plugin_conan_project(basis_plugin_helper.CMakePackage):
    name = "external_plugin"

    version = "master"
    url = "https://CHANGE_ME"
    license = "MIT" # CHANGE_ME
    author = "CHANGE_ME <>"

    description = "external_plugin: CHANGE_ME"
    topics = tuple(['c++', 'CHANGE_ME'])

    options = dict(basis_plugin_helper.CMakePackage.plugin_options, **{
        'example_option': [True, False]
    })

    default_options = dict(basis_plugin_helper.CMakePackage.plugin_default_options, **{
        "example_option": "True",
        "flex_reflect_plugin:shared": "True",
        "boost:no_rtti": "False",
        "boost:no_exceptions": "False",
        "boost:without_python": "True",
        "boost:without_test": "True",
        "boost:without_coroutine": "False",
        "boost:without_stacktrace": "False",
        "boost:without_math": "False",
        "boost:without_wave": "False",
        "boost:without_contract": "False",
        "boost:without_locale": "False",
        "boost:without_random": "False",
        "boost:without_regex": "False",
        "boost:without_mpi": "False",
        "boost:without_timer": "False",
        "boost:without_thread": "False",
        "boost:without_chrono": "False",
        "boost:without_atomic": "False",
        "boost:without_system": "False",
        "boost:without_program_options": "False",
        "boost:without_serialization": "False",
        "boost:without_log": "False",
        "boost:without_type_erasure": "False",
        "boost:without_graph": "False",
        "boost:without_graph_parallel": "False",
        "boost:without_iostreams": "False",
        "boost:without_context": "False",
        "boost:without_fiber": "False",
        "boost:without_filesystem": "False",
        "boost:without_date_time": "False",
        "boost:without_exception": "False",
        "boost:without_container": "False",
    })

    generators = basis_plugin_helper.CMakePackage.plugin_generators

    settings = basis_plugin_helper.CMakePackage.plugin_settings

    exports_sources = tuple(["external_plugin/*"]) \
      + basis_plugin_helper.CMakePackage.plugin_exports_sources

    def build_requirements(self):
        self.plugin_build_requirements()

    def requirements(self):
        self.plugin_requirements()

        self.requires("flex_reflect_plugin/master@conan/stable")

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.parallel = True
        cmake.verbose = True

        cmake.definitions["CONAN_AUTO_INSTALL"] = 'OFF'

        self.plugin_cmake_definitions(cmake)

        self.add_cmake_option(cmake, "EXAMPLE_OPTION", self.options.example_option)

        cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = 'conan_paths.cmake'

        cmake.configure(build_folder=self.plugin_build_subfolder)

        return cmake

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.plugin_package()

    def build(self):
        cmake = self._configure_cmake()
        self.plugin_build(cmake)

    def imports(self):
        self.plugin_imports()

    def package_info(self):
        self.plugin_package_info()
