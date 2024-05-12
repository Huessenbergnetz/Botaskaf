/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "users.h"

using namespace Qt::Literals::StringLiterals;

Users::Users(QObject *parent)
    : Controller(parent)
{
}

void Users::index(Context *c)
{
    c->stash({
        {u"template"_s, u"users/index.html"_s},
    });
}

bool Users::Auto(Context *c)
{
    c->stash({//% "Users"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_users")}});

    return true;
}

#include "moc_users.cpp"
