from conans import ConanFile


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
        ("graphics/6612c7d109@adnn/develop"),
        ("math/ad7a910017@adnn/develop"),

        ("catch2/3.1.0"),
    )

    build_policy = "missing"
    generators = "CMakeDeps", "CMakeToolchain"


    python_requires="shred_conan_base/0.0.3@adnn/stable"
    python_requires_extend="shred_conan_base.ShredBaseConanFile"
