/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "botaskafcutelee.h"

#include "dateformattag.h"
#include "logging.h"

#if defined(QT_DEBUG)
Q_LOGGING_CATEGORY(HBNBOTA_CUTELEE, "hbnzm.cutelee")
#else
Q_LOGGING_CATEGORY(HBNBOAT_CUTELEE, "hbnzm.cutelee", QtInfoMsg)
#endif

using namespace Qt::Literals::StringLiterals;

BotaskafCutelee::BotaskafCutelee(QObject *parent)
    : QObject{parent}
{
}

QHash<QString, AbstractNodeFactory *> BotaskafCutelee::nodeFactories(const QString &name)
{
    Q_UNUSED(name);

    return {{u"hbnbota_dateformat"_s, new DateFormatTag}, {u"hbnbota_dateformat_var"_s, new DateFormatVarTag}};
}

QHash<QString, Filter *> BotaskafCutelee::filters(const QString &name)
{
    Q_UNUSED(name);

    return {};
}

#include "moc_botaskafcutelee.cpp"
