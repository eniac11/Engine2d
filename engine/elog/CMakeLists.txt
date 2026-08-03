include(ExternalProject)
feature_find_package(GTest 1.14.0 RECOMMEND)

#set(NAME GTestDebian)
#set(VERSION 1.14.0-1)
#set(DIRECTORY pool/universe/g/googletest)
#set(DEB_URL https://deb.debian.org/debian)
#set(SOURCE_DIR ${CMAKE_BINARY_DIR}/${NAME}/src)
#set(DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/${NAME}/downloads)
#
#ExternalProject_Add(${NAME}
#        PREFIX ${SOURCE_DIR}
#        DOWNLOAD_COMMAND ""
#        UPDATE_COMMAND ""
#        PATCH_COMMAND ""
#        CONFIGURE_COMMAND ""
#        BUILD_COMMAND ""
#        INSTALL_COMMAND ""
#)
#
#ExternalProject_Add_Step(${NAME} download-orig
#        COMMAND ${CMAKE_COMMAND} -E make_directory ${DOWNLOAD_DIR}
#        COMMAND curl -L
#        -o ${DOWNLOAD_DIR}/${NAME}_${VERSION}.orig.tar.xz
#        ${DEB_URL}/${DIRECTORY}/${NAME}/${NAME}_${VERSION}.orig.tar.xz
#        DEPENDEES download
#)
#
#ExternalProject_Add_Step(${NAME} download-debian
#        COMMAND curl -L
#        -o ${DOWNLOAD_DIR}/${NAME}_${VERSION}.debian.tar.xz
#        ${DEB_URL}/${DIRECTORY}/${NAME}/${NAME}_${VERSION}.debian.tar.xz
#
#        DEPENDEES download-orig
#)
#
#ExternalProject_Add_Step(${NAME} extract-orig
#        COMMAND ${CMAKE_COMMAND} -E tar xf ${DOWNLOAD_DIR}/${NAME}_${VERSION}.orig.tar.xz
#        WORKING_DIRECTORY ${SOURCE_DIR}
#        DEPENDEES download-debian
#)
#
#ExternalProject_Add_Step(${NAME} extract-debian
#        COMMAND ${CMAKE_COMMAND} -E tar xf $${DOWNLOAD_DIR}/${NAME}_${VERSION}.debian.tar.xz
#        WORKING_DIRECTORY ${SOURCE_DIR}
#        DEPENDEES extract-orig
#)
#
#ExternalProject_Add_Step(${NAME} apply-patches
#        COMMAND dpkg-source --before-build .
#        WORKING_DIRECTORY ${SOURCE_DIR}
#        DEPENDEES extract-debian
#)

feature_find_package(PkgConfig)

option(ENGINE_ELOG_USE_SOURCE_LOCATION "Use std::source_location instead of macros" ON)

pkg_check_modules(SYSTEMD IMPORTED_TARGET libsystemd)

defer_on_missing_required_packages()

add_library(engine_elog)
target_sources(engine_elog
    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include ${CMAKE_CURRENT_BINARY_DIR}/include
    FILES
        include/engine/elog.h
        include/engine/string_utils.h
        include/engine/enum_utils.h
        ${CMAKE_CURRENT_BINARY_DIR}/include/engine/elogConfig.h
    PRIVATE
        src/elog.cpp
)
if(TARGET PkgConfig::SYSTEMD)
  set(HAVE_JOURNAL 1)
  target_link_libraries(engine_elog PRIVATE PkgConfig::SYSTEMD)
endif()
configure_file(elogConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/engine/elogConfig.h)

if (ENGINE_ELOG_USE_SOURCE_LOCATION)
    target_compile_definitions(engine_elog PUBLIC ELOG_USE_SOURCE_LOCATION)
endif()
#if (GTEST_FOUND)
include(GoogleTest)

add_executable(elog_tests_suite
        tests/test_basic_logging.cpp
        tests/test_log_rules.cpp
        tests/test_log_registry.cpp
        tests/test_split.cpp
)
target_link_libraries(elog_tests_suite PRIVATE GTest::gtest_main engine_elog)

#target_compile_options(elog_tests_suite PRIVATE -coverage)
#target_link_options(elog_tests_suite PRIVATE -coverage)

gtest_discover_tests(elog_tests_suite)
#endif()


include(GNUInstallDirs)


install(TARGETS engine_elog EXPORT EngineElogTargets
        LIBRARY
        FILE_SET public_headers
)

install(EXPORT EngineElogTargets
        FILE EngineElogTargets.cmake
        NAMESPACE engine::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine
)
