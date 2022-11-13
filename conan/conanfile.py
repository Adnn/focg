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
        ("graphics/aa960a2385@adnn/develop"),
        ("math/541fce5f6a@adnn/develop"),

        ("catch2/3.1.0"),
    )

    build_policy = "missing"
    generators = "CMakeDeps", "CMakeToolchain"


    python_requires="shred_conan_base/0.0.5@adnn/stable"
    python_requires_extend="shred_conan_base.ShredBaseConanFile"
