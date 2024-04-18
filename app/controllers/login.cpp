/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "login.h"

Login::Login(QObject *parent)
    : Controller(parent)
{
}

void Login::index(Context *c)
{
    c->response()->body() = "Login to Botaskaf!";
}

#include "moc_login.cpp"
