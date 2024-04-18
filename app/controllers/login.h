/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_LOGIN_H
#define HBNBOTA_LOGIN_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Login final : public Controller // NOLINT(cppcoreguidelines-special-member-functions)
{
    Q_OBJECT
    Q_DISABLE_COPY(Login)
public:
    explicit Login(QObject *parent = nullptr);
    ~Login() override = default;

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);
};

#endif // HBNBOTA_LOGIN_H
