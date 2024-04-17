# SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: AGPL-3.0-or-later

function(hbnbota_test _testname)
    add_executable(${_testname}_exec ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_link_libraries(${_testname}_exec Qt::Test Botaskaf)
    target_include_directories(${_testname}_exec
        PRIVATE
            ${CMAKE_SOURCE_DIR}/app
            ${CMAKE_BINARY_DIR}/app
    )
endfunction(hbnbota_test _testname)

hbnbota_test(testuser)
