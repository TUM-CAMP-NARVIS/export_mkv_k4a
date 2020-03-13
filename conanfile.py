from conans import ConanFile, CMake
from conans import tools
from conans.tools import os_info, SystemPackageTool
import os, sys
import sysconfig
from io import StringIO

class ExportMKVConan(ConanFile):
    name = "export_mkv_k4a"
    version = "0.1.0"

    description = "export_mkv_k4a"
    url = "https://github.com/TUM-CAMP-NARVIS/export_mkv_k4a"
    license = "GPL"

    short_paths = True
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "virtualrunenv"

    requires = (
        "opencv/3.4.8@camposs/stable",
        "magnum/2019.10@camposs/stable",
        "corrade/2019.10@camposs/stable",
        "kinect-azure-sensor-sdk/1.3.0@camposs/stable",
        "bzip2/1.0.6@camposs/stable",
         )

    default_options = {
        "opencv:shared": True,
        "magnum:with_anyimageimporter": True,
        "magnum:with_tgaimporter": True,
        "magnum:with_anysceneimporter": True,
        "magnum:with_gl_info": True,
        "magnum:with_objimporter": True,
        "magnum:with_tgaimageconverter": True,
        "magnum:with_imageconverter": True,
        "magnum:with_anyimageconverter": True,
        "magnum:with_sdl2application": True,
        "magnum:with_eglcontext": False,
        "magnum:with_windowlesseglapplication": False,
        "magnum:target_gles": False,
        "magnum:with_opengltester": True,
    }

    # all sources are deployed with the package
    exports_sources = "cmake/*", "include/*", "src/*", "CMakeLists.txt"

    def configure(self):

        if self.settings.os == "Linux":
            self.options["opencv"].with_gtk = True

        if self.settings.os == "Macos":
            self.options['magnum-extras'].with_ui_gallery = False
            self.options['magnum-extras'].with_player = False
            self.options['magnum-plugins'].with_tinygltfimporter = False


        if self.settings.os == "Windows":
            self.options['magnum'].with_windowlesswglapplication = True

    def imports(self):
        self.copy(src="bin", pattern="*.dll", dst="./bin") # Copies all dll files from packages bin folder to my "bin" folder
        self.copy(src="lib", pattern="*.dll", dst="./bin") # Copies all dll files from packages bin folder to my "bin" folder
        self.copy(src="lib", pattern="*.dylib*", dst="./lib") # Copies all dylib files from packages lib folder to my "lib" folder
        self.copy(src="lib", pattern="*.so*", dst="./lib") # Copies all so files from packages lib folder to my "lib" folder
        self.copy(src="lib", pattern="*.a", dst="./lib") # Copies all static libraries from packages lib folder to my "lib" folder
        self.copy(src="bin", pattern="*", dst="./bin") # Copies all applications

    def _cmake_configure(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        return cmake
       
    def build(self):
        cmake = self._cmake_configure()
        cmake.build()

    def package(self):
        cmake = self._cmake_configure()
        cmake.install()
