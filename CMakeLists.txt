# SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

project(Botaskaf
  VERSION 0.0.1
  DESCRIPTION "Cutelyst based endpoint for contact forms on static websites."
  HOMEPAGE_URL https://github.com/Huessenbergnetz/Botaskaf
  LANGUAGES CXX
)

find_package(Qt6 6.4.0 COMPONENTS Core Network Sql LinguistTools REQUIRED)
find_package(Cutelyst4Qt6 REQUIRED)
find_package(Cutelee6Qt6 REQUIRED)
find_package(Cutelyst4Qt6Forms REQUIRED)
find_package(Cutelyst4Qt6Botan REQUIRED)
find_package(FirfuoridaQt6 REQUIRED)
find_package(SimpleMail2Qt6 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(Botan REQUIRED IMPORTED_TARGET botan-2)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build in shared lib mode" ON)
option(ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF)
option(BUILD_TESTS "Build the Botaskaf tests" OFF)

set(HBNBOTA_APPDIR ${CMAKE_INSTALL_DATADIR}/botaskaf CACHE PATH "Directory to install Botaskaf data files into")
set(HBNBOTA_TEMPLATESDIR ${HBNBOTA_APPDIR}/templates CACHE PATH "Directory to install Botaskaf template files into")
set(HBNBOTA_TRANSLATIONSDIR ${HBNBOTA_APPDIR}/translations CACHE PATH "Directory to install Botaskaf translation files into")
set(HBNBOTA_FORMSDIR ${HBNBOTA_APPDIR}/forms CACHE PATH "Directory to install Botaskaf form files into")
set(HBNBOTA_CONFFILE ${CMAKE_INSTALL_SYSCONFDIR}/botaskaf.ini CACHE FILEPATH "Path to the main configuration file")

GNUInstallDirs_get_absolute_install_dir(HBNBOTA_FULL_TRANSLATIONSDIR HBNBOTA_TRANSLATIONSDIR DATADIR)
GNUInstallDirs_get_absolute_install_dir(HBNBOTA_FULL_TEMPLATESDIR HBNBOTA_TEMPLATESDIR DATADIR)
GNUInstallDirs_get_absolute_install_dir(HBNBOTA_FULL_FORMSDIR HBNBOTA_FORMSDIR DATADIR)
GNUInstallDirs_get_absolute_install_dir(HBNBOTA_FULL_CONFFILE HBNBOTA_CONFFILE DATADIR)

configure_file(testApp.sh.in testApp.sh
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    @ONLY
)

add_compile_definitions(
    QT_NO_CAST_TO_ASCII
    QT_NO_CAST_FROM_ASCII
    QT_STRICT_ITERATORS
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_USE_QSTRINGBUILDER
    HBNBOTA_VERSION="${PROJECT_VERSION}"
    HBNBOTA_TRANSLATIONSDIR="${HBNBOTA_FULL_TRANSLATIONSDIR}"
    HBNBOTA_TEMPLATESDIR="${HBNBOTA_FULL_TEMPLATESDIR}"
    HBNBOTA_FORMSDIR="${HBNBOTA_FULL_FORMSDIR}"
)

if(WIN32)
    if(MSVC)
        add_compile_definitions(
            _SCL_SECURE_NO_WARNINGS
            _CRT_SECURE_NO_DEPRECATE
        )
    endif()
endif()

if(ENABLE_MAINTAINER_CFLAGS)
    add_compile_definitions(QT_DEPRECATED_WARNINGS)

    if(NOT MSVC)
        add_compile_options(
            -Wall
            -Wextra
            -Wcast-align
            -Wno-uninitialized
            -Wempty-body
            -Wformat-security
            -Wformat
            -Winit-self
            -Wunused-variable
        )
    endif()

    if(APPLE)
        add_compile_options(-Wweak-vtables)
    endif()
endif()

# Enable -Werror
if(NOT MSVC OR IS_CLANG_BUILD) # clang-cl accepts these too
    add_compile_options(-Werror -Wundef -Wno-error=deprecated-declarations)
endif()

# Set configuration key names and default values

set(HBNBOTA_CONF_DB "database")
set(HBNBOTA_CONF_DB_TYPE "type")
set(HBNBOTA_CONF_DB_TYPE_DEFVAL "QSQLITE")
set(HBNBOTA_CONF_DB_HOST "host")
set(HBNBOTA_CONF_DB_HOST_DEFVAL "localhost")
set(HBNBOTA_CONF_DB_USER "user")
set(HBNBOTA_CONF_DB_USER_DEFVAL "botaskaf")
set(HBNBOTA_CONF_DB_PASS "password")
set(HBNBOTA_CONF_DB_NAME "name")
set(HBNBOTA_CONF_DB_NAME_DEFVAL "")
set(HBNBOTA_CONF_DB_PORT "port")
set(HBNBOTA_CONF_DB_PORT_DEFVAL 3306)

set(HBNBOTA_CONF_CORE "core")
set(HBNBOTA_CONF_CORE_SETUPTOKEN "setuptoken")
set(HBNBOTA_CONF_CORE_TEMPLATE "template")
set(HBNBOTA_CONF_CORE_TEMPLATE_DEFVAL "botaskaf")
set(HBNBOTA_CONF_CORE_STATICPLUGIN "staticplugin")
set(HBNBOTA_CONF_CORE_STATICPLUGIN_DEFVAL "simple")
set(HBNBOTA_CONF_CORE_CACHE "cache")
set(HBNBOTA_CONF_CORE_CACHE_DEFVAL "none")
set(HBNBOTA_CONF_CORE_SESSIONSTORE "sessionstore")
set(HBNBOTA_CONF_CORE_SESSIONSTORE_DEFVAL "file")

set(HBNBOTA_CONF_DEFAULTS "defaults")
set(HBNBOTA_CONF_DEFAULTS_LANG "language")
set(HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL "en_US")
set(HBNBOTA_CONF_DEFAULTS_TZ "timezone")
set(HBNBOTA_CONF_DEFAULTS_TZ_DEFVAL "UTC")

add_subdirectory(app)
add_subdirectory(templates)

if (BUILD_TESTS)
    find_package(Qt6 COMPONENTS Test REQUIRED)
    enable_testing()
    add_subdirectory(tests)
endif (BUILD_TESTS)
