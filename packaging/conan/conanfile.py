# Conan 2.x recipe for d4np-c (roadmap M8.3; see ADR-0005).
#
# The recipe lives in packaging/conan/ but builds the library at the repository root, so it
# exports the needed sources from two levels up. Create/test the package with:
#
#     conan create packaging/conan --build=missing
#
# SPDX-License-Identifier: MIT
import os
import re

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load


class D4npcConan(ConanFile):
    name = "d4np-c"
    license = "MIT"
    author = "Daniel Polo <daniel.polo@sbe.it>"
    url = "https://github.com/danielPoloWork/egl-utils-c"
    homepage = url
    description = (
        "Dependency-free, high-performance C11 systems utility library "
        "(allocators, containers, concurrency, strings, I/O, diagnostics)."
    )
    topics = ("c11", "allocators", "data-structures", "concurrency", "utilities", "no-dependencies")
    package_type = "static-library"
    settings = "os", "arch", "compiler", "build_type"
    options = {"fPIC": [True, False]}
    default_options = {"fPIC": True}

    def _root(self):
        # The repository root, two levels above packaging/conan/.
        return os.path.abspath(os.path.join(self.recipe_folder, "..", ".."))

    def set_version(self):
        header = load(self, os.path.join(self._root(), "d4np", "core", "version.h"))
        parts = [re.search(rf"D4NP_VERSION_{k}\s+(\d+)", header).group(1)
                 for k in ("MAJOR", "MINOR", "PATCH")]
        self.version = ".".join(parts)

    def export_sources(self):
        root = self._root()
        copy(self, "CMakeLists.txt", root, self.export_sources_folder)
        copy(self, "LICENSE", root, self.export_sources_folder)
        copy(self, "README.md", root, self.export_sources_folder)
        copy(self, "d4np/*", root, self.export_sources_folder)
        copy(self, "cmake/*", root, self.export_sources_folder)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        # Pure C library: drop the C++ settings Conan adds by default.
        self.settings.rm_safe("compiler.libcxx")
        self.settings.rm_safe("compiler.cppstd")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["D4NP_BUILD_TESTS"] = "OFF"
        tc.cache_variables["D4NP_BUILD_BENCH"] = "OFF"
        tc.cache_variables["D4NP_INSTALL"] = "ON"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.libs = ["d4np"]
        # Match the CMake package this project installs so find_package(d4np-c) keeps working.
        self.cpp_info.set_property("cmake_file_name", "d4np-c")
        self.cpp_info.set_property("cmake_target_name", "d4np::d4np")
        if self.settings.os in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs.append("pthread")
