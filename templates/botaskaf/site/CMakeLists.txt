# SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: AGPL-3.0-or-later

target_sources(botaskaftemplate
    PRIVATE
        base.html
        footer.html
        header.html
        index.html
        wrapper.html
        login.html
        error.html
)

add_subdirectory(setup)
add_subdirectory(users)
add_subdirectory(parts)
add_subdirectory(mysettings)
