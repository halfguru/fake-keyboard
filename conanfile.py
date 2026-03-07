from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout


class FakeKeyboardRecipe(ConanFile):
    name = "fake-keyboard"
    version = "0.1.0"

    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("spdlog/1.17.0")
        self.requires("nlohmann_json/3.12.0")

    def build_requirements(self):
        self.test_requires("catch2/3.13.0")
        self.tool_requires("ninja/1.13.2")
        self.tool_requires("cmake/[>=3.25]")
        self.tool_requires("pkgconf/2.5.1")

    exports_sources = (
        "CMakeLists.txt",
        "src/*",
        "tests/*",
        "config/*",
        "examples/*",
        "tools/*",
    )

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def test(self):
        cmake = CMake(self)
        cmake.test()
