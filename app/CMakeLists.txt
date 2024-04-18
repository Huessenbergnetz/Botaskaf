# SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: AGPL-3.0-or-later

add_library(Botaskaf SHARED)

target_sources(Botaskaf
    PRIVATE
        botaskaf.cpp
        botaskaf.h
        logging.h
        confignames.h.in
        settings.h
        settings.cpp
        userauthstoresql.cpp
        userauthstoresql.h
)

add_subdirectory(controllers)
add_subdirectory(objects)
add_subdirectory(migrations)

target_link_libraries(Botaskaf
    PRIVATE
        Qt::Core
        Qt::Network
        Qt::Sql
        SimpleMail::Core
        FirfuoridaQt6::Core
        Cutelyst::Core
        Cutelyst::Session
        Cutelyst::Authentication
        Cutelyst::StatusMessage
        Cutelyst::View::Cutelee
        Cutelyst::StaticSimple
        Cutelyst::StaticCompressed
        Cutelyst::Utils::Validator
        Cutelyst::Utils::Sql
        Cutelyst::Utils::Pagination
        Cutelyst::Utils::LangSelect
        Cutelyst::Memcached
        Cutelyst::MemcachedSessionStore
        Cutelyst::CSRFProtection
        Cutelee::Templates
        CutelystBotan::Core
)

configure_file(confignames.h.in confignames.h)

install(TARGETS Botaskaf DESTINATION ${CMAKE_INSTALL_LIBDIR}/cutelyst${Cutelyst3Qt6_VERSION_MAJOR}-qt6-apps)
