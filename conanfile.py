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
        self.requires("tracy/0.13.1")

    def configure(self):
        if self.settings.os == "Windows":
            self.options["libcurl"].with_ssl = "schannel"

    def layout(self):
        cmake_layout(self)
