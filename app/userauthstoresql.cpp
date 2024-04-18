/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "userauthstoresql.h"

#include "logging.h"
#include "objects/user.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Request>

#include <QSqlError>
#include <QSqlQuery>

using namespace Qt::Literals::StringLiterals;

#if defined(QT_DEBUG)
Q_LOGGING_CATEGORY(HBNBOTA_AUTHN, "hbnbota.authn")
Q_LOGGING_CATEGORY(HBNBOTA_AUTHZ, "hbnbota.authz")
#else
Q_LOGGING_CATEGORY(HBNBOTA_AUTHN, "hbnbota.authn", QtInfoMsg)
Q_LOGGING_CATEGORY(HBNBOTA_AUTHZ, "hbnbota.authz", QtInfoMsg)
#endif

UserAuthStoreSql::UserAuthStoreSql()
    : AuthenticationStore()
{
}

AuthenticationUser UserAuthStoreSql::findUser(Context *c, const ParamsMultiMap &userinfo)
{
    const QString email = userinfo.value(u"email"_s).toLower();

    QSqlQuery q =
        CPreparedSqlQueryThreadFO(u"SELECT id, type, password FROM users WHERE email = :email"_s);
    q.bindValue(u":email"_s, email);

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(HBNBOTA_AUTHN) << "Failed to execute database query to get user with email"
                                  << email << "from the database:" << q.lastError().text();
        return {};
    }

    if (Q_UNLIKELY(!q.next())) {
        qCWarning(HBNBOTA_AUTHN).noquote().nospace()
            << "Login failed: email address " << email
            << " not found. IP: " << c->req()->addressString();
        return {};
    }

    const auto userId   = q.value(0);
    const auto userType = static_cast<User::Type>(q.value(1).toInt());

    if (userType <= User::Disabled) {
        qCWarning(HBNBOTA_AUTHN).noquote().nospace()
            << "Login failed: user with email address " << email
            << " is disabled: IP: " << c->req()->addressString();
        return {};
    }

    AuthenticationUser user(userId);
    user.insert(u"email"_s, email);
    user.insert(u"password"_s, q.value(2));
    user.insert(u"type"_s, userType);

    return user;
}
