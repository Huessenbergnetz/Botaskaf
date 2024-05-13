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
        qtimezonevariant_p.h
)

add_subdirectory(controllers)
add_subdirectory(objects)
add_subdirectory(migrations)
add_subdirectory(forms)
add_subdirectory(cutelee)

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
        CutelystForms::Core
)

configure_file(confignames.h.in confignames.h)

install(TARGETS Botaskaf DESTINATION ${CMAKE_INSTALL_LIBDIR}/cutelyst${Cutelyst3Qt6_VERSION_MAJOR}-qt6-apps)

set(Botaskaf_TS_FILES
    ${CMAKE_SOURCE_DIR}/translations/botaskaf.en.ts
    ${CMAKE_SOURCE_DIR}/translations/botaskaf.de.ts
)

set_source_files_properties(${Botaskaf_TS_FILES}
    PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/translations"
)

qt_add_lupdate(Botaskaf
    TS_FILES ${Botaskaf_TS_FILES}
    OPTIONS -no-obsolete -locations none -source-language en -tr-function-alias qsTrId=cTrId
)

qt_add_lrelease(Botaskaf
    TS_FILES ${Botaskaf_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE Botaskaf_QM_FILES
    OPTIONS -idbased
)

install(FILES ${Botaskaf_QM_FILES} DESTINATION ${HBNBOTA_TRANSLATIONSDIR})
