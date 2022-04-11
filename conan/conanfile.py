from conans import ConanFile, errors, tools
from conan.tools.cmake import CMake

from os import path


class FocgConan(ConanFile):
    """ Conan recipe for Focg """

    name = "focg"
    license = "MIT License"
    url = "https://github.com/Adnn/focg"
    description = "Implementation for programs and exercises from 'Fundamentals of Computer Graphics' book."
    #topics = ("", "", ...)
    settings = ("os", "compiler", "build_type", "arch")
    options = {
        "shared": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "build_tests": False,
    }

    requires = (
        ("graphics/115fc983cf@adnn/develop"),
        ("math/0541d9cdd9@adnn/develop"),
    )

    build_requires = ("cmake/3.20.4",)

    build_policy = "missing"
    generators = "cmake_paths", "cmake_find_package", "CMakeToolchain"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto",
        "submodule": "recursive",
    }


    def _generate_cmake_configfile(self):
        """ Generates a conanuser_config.cmake file which includes the file generated by """
        """ cmake_paths generator, and forward the remaining options to CMake. """
        with open("conanuser_config.cmake", "w") as config:
            config.write("message(STATUS \"Including user generated conan config.\")\n")
            # avoid path.join, on Windows it outputs '\', which is a string escape sequence.
            config.write("include(\"{}\")\n".format("${CMAKE_CURRENT_LIST_DIR}/conan_paths.cmake"))
            config.write("set({} {})\n".format("BUILD_tests", self.options.build_tests))


    def generate(self):
        self._generate_cmake_configfile()


    def configure(self):
        tools.check_min_cppstd(self, "17")


    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake


    def build(self):
        cmake = self._configure_cmake()
        cmake.build()


    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
