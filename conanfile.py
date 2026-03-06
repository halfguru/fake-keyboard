from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout


class FakeKeyboardRecipe(ConanFile):
    name = "fake-keyboard"
    version = "0.1.0"
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    
    # Dependencies from Conan Center
    requirements = [
        "spdlog/1.12.0",
        "nlohmann_json/3.11.2",
        "catch2/3.5.0"
    ]
    
    # Sources
    exports_sources = "CMakeLists.txt", "src/*", "include/*"
    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def test(self):
        cmake = CMake(self)
        cmake.test()
