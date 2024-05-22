/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_SETTINGS_H
#define HBNBOTA_SETTINGS_H

#include <CutelystForms/option.h>

#include <QLocale>
#include <QString>
#include <QTimeZone>

namespace Cutelyst {
class Context;
}

namespace Settings {
Q_NAMESPACE

enum class StaticPlugin : int { None = 0, Simple, Compressed };
Q_ENUM_NS(StaticPlugin)

enum class Cache : int { None = 0, Memcached };
Q_ENUM_NS(Cache)

enum class SessionStore : int { File = 0, Memcached };
Q_ENUM_NS(SessionStore)

bool load(const QVariantMap &core, const QVariantMap &defaults);

void loadSupportedLocales(const QVector<QLocale> &locales);

/*!
 * \brief The setup token used to initialize the setup.
 *
 * \par Section
 * core
 *
 * \par Key
 * setuptoken
 */
QString setupToken();

/*!
 * \brief The name of the template to use.
 *
 * \par Section
 * core
 *
 * \par Key
 * template
 */
QString tmpl();

/*!
 * \brief Full path to the used template.
 */
QString tmplPath();

/*!
 * \brief Full \a path below the current template’s path.
 */
QString tmplPath(QStringView path);

QString tmplIcon(const QString &name);

/*!
 * \brief The main site name.
 */
QString siteName();

/*!
 * \brief The static plugin to use.
 */
StaticPlugin staticPlugin();

/*!
 * \brief The cache type to use.
 */
Cache cache();

/*!
 * \brief The session store to use.
 */
SessionStore sessionStore();

QLocale defLocale();

QTimeZone defTimeZone();

QList<CutelystForms::Option *> supportedLocales(Cutelyst::Context *c, const QLocale &selected, QObject *parent = nullptr);

QStringList allowedLocaleIds();

QList<CutelystForms::Option *> supportedTimeZones(const QByteArray &selected, QObject *parent = nullptr);

QStringList allowedTimeZoneIds();

QList<CutelystForms::Option *> supportedSmtpAuthMethods(Cutelyst::Context *c, const QString &selected);

QStringList allowedSmtpAuthMethods();

QList<CutelystForms::Option *> supportedSmtpEncryption(Cutelyst::Context *c, const QString &selected);

QStringList allowedSmtpEncryption();

QList<CutelystForms::Option *> supportedSenderTypes(Cutelyst::Context *c, const QString &selected);

QStringList allowedSenderTypes();
} // namespace Settings

#endif // HBNBOTA_SETTINGS
