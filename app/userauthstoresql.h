/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_USERAUTHSTORESQL_H
#define HBNBOTA_USERAUTHSTORESQL_H

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>

using namespace Cutelyst;

class UserAuthStoreSql final : public AuthenticationStore
{
    Q_DISABLE_COPY(UserAuthStoreSql)
public:
    UserAuthStoreSql();
    ~UserAuthStoreSql() override = default;

    AuthenticationUser findUser(Context *c, const ParamsMultiMap &userinfo) override;
};

#endif // HBNBOTA_USERAUTHSTORESQL_H
