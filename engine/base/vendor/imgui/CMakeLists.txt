project(dearimgui VERSION 1.92.9)
add_library(imgui)
target_sources(imgui
    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
        include/imgui/imgui.h
        include/imgui/imconfig.h
    PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    BASE_DIRS src
    FILES
        src/imgui_internal.h
        src/imstb_rectpack.h
        src/imstb_textedit.h
        src/imstb_truetype.h
    PRIVATE
        src/imgui.cpp
        src/imgui_demo.cpp
        src/imgui_draw.cpp
        src/imgui_tables.cpp
        src/imgui_widgets.cpp
)
target_include_directories(imgui PRIVATE include/imgui)

add_library(imgui::imgui ALIAS imgui)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
        ImGuiConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/dearimgui/dearimguiConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dearimgui)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/dearimgui/dearimguiVersion.cmake
        COMPATIBILITY SameMajorVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dearimgui/dearimguiConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/dearimgui/dearimguiVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dearimgui)

install(TARGETS imgui EXPORT DearImGuiTargets
        LIBRARY
        FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT DearImGuiTargets
        FILE DearImGuiTargets.cmake
        NAMESPACE imgui::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dearimgui
)

