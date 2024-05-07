/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "logout.h"

#include "logging.h"
#include "objects/user.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>

Logout::Logout(QObject *parent)
    : Controller(parent)
{
}

void Logout::index(Context *c)
{
    qCInfo(HBNBOTA_AUTHN) << User::fromStash(c) << "logged out";

    Authentication::logout(c);
    c->res()->redirect(c->uriFor(QStringLiteral("/login")));
}

#include "moc_logout.cpp"
