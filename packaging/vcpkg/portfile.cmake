# vcpkg port for d4np-c (roadmap M8.3; see ADR-0005).
#
# d4np-c is a static C library with no third-party dependencies (only the platform threads
# library), so the port is a plain CMake configure/install/fixup.
#
# NOTE: SHA512 below is a placeholder. No release tag exists yet (see the project's integration
# status). On the first `vX.Y.Z` tag, set REF to that tag and replace the SHA512 with the value
# vcpkg prints on a hash mismatch (run the port once with the placeholder to obtain it).

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO danielPoloWork/egl-utils-c
    REF "v${VERSION}"
    SHA512 0
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DD4NP_BUILD_TESTS=OFF
        -DD4NP_BUILD_BENCH=OFF
        -DD4NP_INSTALL=ON
)

vcpkg_cmake_install()

# Move the exported CMake package config to the vcpkg-standard share/ location.
vcpkg_cmake_config_fixup(PACKAGE_NAME d4np-c CONFIG_PATH lib/cmake/d4np-c)

# A header + static-lib package ships no debug headers.
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
