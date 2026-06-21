# vcpkg port for d4np-c (roadmap M8.3; see ADR-0005).
#
# d4np-c is a static C library with no third-party dependencies (only the platform threads
# library), so the port is a plain CMake configure/install/fixup.
#
# REF tracks the port version (vcpkg.json), so it resolves to the `v${VERSION}` git tag. The
# SHA512 is over GitHub's source archive for that tag (the implementation-only export-ignore
# bundle). To refresh on a new release: bump vcpkg.json, then replace the SHA512 with the value
# vcpkg prints on a hash mismatch (or `curl -sL .../archive/v<ver>.tar.gz | sha512sum`).

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO danielPoloWork/egl-utils-c
    REF "v${VERSION}"
    SHA512 7c9eccb0cf8263297b8589c1dc1404c7daef698a8ffaf2b3776c80059591a6323c7c9fd587f8a3e5ddf7920dedbfd5e3bf3f0978605c1af4755c8611eb6f7cd0
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
