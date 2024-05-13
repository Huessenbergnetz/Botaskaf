/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "users.h"

#include "logging.h"
#include "objects/error.h"
#include "objects/user.h"

using namespace Qt::Literals::StringLiterals;

Users::Users(QObject *parent)
    : Controller(parent)
{
}

void Users::index(Context *c)
{
    if (User::fromStash(c).type() < User::Administrator) {
        //% "Sorry, but you do not have access authorization for this area."
        Error::toStash(c, Response::Forbidden, c->qtTrId("hbnbota_error_general_forbidden"), true);
        qCWarning(HBNBOTA_AUTHZ) << User::fromStash(c) << "tried to access restricted area" << c->req()->uri().path();
        return;
    }

    Error e;
    const auto users = User::list(c, e);
    if (e) {
        e.toStash(c);
    } else {
        c->setStash(u"users_labels"_s, QVariant::fromValue<QMap<QString, QString>>(User::labels(c)));
    }

    c->stash({{u"template"_s, u"users/index.html"_s}, {u"users"_s, QVariant::fromValue<QList<User>>(users)}});
}

bool Users::Auto(Context *c)
{
    c->stash({//% "Users"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_users")}});

    return true;
}

#include "moc_users.cpp"
