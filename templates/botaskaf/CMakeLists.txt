# SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
# SPDX-License-Identifier: AGPL-3.0-or-later

add_custom_target(botaskaftemplate
    SOURCES
        package.json
        webpack.mix.js
)

add_subdirectory(assets)
add_subdirectory(site)
