/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "root.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>

using namespace Qt::Literals::StringLiterals;

Root::Root(QObject *parent)
    : Controller(parent)
{
}

void Root::index(Context *c)
{
    c->response()->body() = "Welcome to Botaskaf!";
}

void Root::defaultPage(Context *c)
{
    c->response()->body() = "Page not found!";
    c->response()->setStatus(Response::NotFound);
}

bool Root::Auto(Context *c)
{
    if (c->controllerName() == "Login"_L1) {
        return true;
    }

    if (c->controllerName() == "Logout"_L1) {
        return true;
    }

    const auto user = Authentication::user(c);

    if (Q_UNLIKELY(user.isNull())) {
        c->res()->redirect(c->uriFor(u"/login"_s));
        return false;
    }

    return true;
}
