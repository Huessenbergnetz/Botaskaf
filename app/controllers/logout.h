/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_LOGOUT_H
#define HBNBOTA_LOGOUT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Logout final : public Controller
{
    Q_OBJECT
    Q_DISABLE_COPY(Logout)
public:
    explicit Logout(QObject *parent = nullptr);
    ~Logout() override = default;

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);
};

#endif // HBNBOTA_LOGOUT_H
