from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMakeDeps

class SaveManager(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def requirements(self):
        self.requires("libcurl/8.19.0")
        self.requires("libzip/1.11.4")
        self.requires("libssh2/1.11.1")
        self.requires("imgui/1.92.6")
        self.requires("glfw/3.4")
        self.requires("opengl/system")
        self.requires("nlohmann_json/3.12.0")
        self.requires("openssl/3.6.1")
        self.requires("glad/2.0.8")
        self.requires("stb/cci.20230920")
        self.requires("sol2/3.5.0")
        self.requires("spdlog/1.17.0")
        self.requires("yaml-cpp/0.9.0")
        self.requires("cli11/2.6.2")

    def configure(self):
        if self.settings.os == "Windows":
            self.options["libcurl"].with_ssl = "schannel"
        self.options["cli11"].header_only = True 

    def layout(self):
        cmake_layout(self)
