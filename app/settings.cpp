/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "settings.h"

#include "confignames.h"
#include "logging.h"

#include <Cutelyst/Context>

#include <QFile>
#include <QFileInfo>
#include <QGlobalStatic>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QTimeZone>
#include <QVariantMap>
#include <QWriteLocker>

using namespace Qt::Literals::StringLiterals;

#if defined(QT_DEBUG)
Q_LOGGING_CATEGORY(HBNBOTA_SETTINGS, "hbnbota.settings")
#else
Q_LOGGING_CATEGORY(HBNBOTA_SETTINGS, "hbnbota.settings", QtInfoMsg)
#endif

struct SettingVals {
    mutable QReadWriteLock lock{QReadWriteLock::Recursive};

    QVector<QLocale> supportedLocales;

    QString setupToken;
    QString tmpl    = QStringLiteral(HBNBOTA_CONF_CORE_TEMPLATE_DEFVAL);
    QString tmplDir = QStringLiteral(HBNBOTA_TEMPLATESDIR);

    QHash<QString, QString> tmplIcons;

    QLocale defLocale{QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL)};
    QTimeZone defTimeZone{QByteArrayLiteral(HBNBOTA_CONF_DEFAULTS_TZ_DEFVAL)};

    Settings::StaticPlugin staticPlugin{Settings::StaticPlugin::Simple};
    Settings::Cache cache{Settings::Cache::None};
    Settings::SessionStore sessionStore{Settings::SessionStore::File};

    bool loaded{false};
    bool localesLoaded{false};
};

Q_GLOBAL_STATIC(SettingVals, cfg) // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

bool loadCore(const QVariantMap &core)
{
    cfg->setupToken = core.value(QStringLiteral(HBNBOTA_CONF_CORE_SETUPTOKEN)).toString();

    cfg->tmpl =
        core.value(QStringLiteral(HBNBOTA_CONF_CORE_TEMPLATE), QStringLiteral(HBNBOTA_CONF_CORE_TEMPLATE_DEFVAL)).toString();
    if (cfg->tmpl.startsWith('/'_L1)) {
        auto fullPathParts = cfg->tmpl.split('/'_L1, Qt::SkipEmptyParts);
        cfg->tmpl          = fullPathParts.takeLast();
        cfg->tmplDir       = '/'_L1 + fullPathParts.join('/'_L1);
    }

    const QFileInfo tmplDirFi{cfg->tmplDir + '/'_L1 + cfg->tmpl};
    if (Q_UNLIKELY(!tmplDirFi.exists())) {
        qCCritical(HBNBOTA_SETTINGS) << "Template directory" << tmplDirFi.absoluteFilePath() << "does not exist";
        return false;
    }

    if (Q_UNLIKELY(!tmplDirFi.isDir())) {
        qCCritical(HBNBOTA_SETTINGS) << "Template directory path" << tmplDirFi.absoluteFilePath()
                                     << "does not point to a directory";
        return false;
    }

    QFile tmplMetaDataFile(cfg->tmplDir + '/'_L1 + cfg->tmpl + "/metadata.json"_L1);
    if (Q_UNLIKELY(!tmplMetaDataFile.exists())) {
        qCCritical(HBNBOTA_SETTINGS) << "Can not find template meta data file at" << tmplMetaDataFile.fileName();
        return false;
    }

    if (Q_UNLIKELY(!tmplMetaDataFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text))) {
        qCCritical(HBNBOTA_SETTINGS) << "Failed to open template meta data file:" << tmplMetaDataFile.errorString();
        return false;
    }

    QJsonParseError jpe;
    const QJsonDocument json = QJsonDocument::fromJson(tmplMetaDataFile.readAll(), &jpe);

    if (Q_UNLIKELY(jpe.error != QJsonParseError::NoError)) {
        qCCritical(HBNBOTA_SETTINGS) << "Failed to parse template meta data JSON file:" << jpe.errorString();
        return false;
    }

    const auto mdo = json.object();
    if (Q_UNLIKELY(mdo.isEmpty())) {
        qCCritical(HBNBOTA_SETTINGS) << "Template meta data file is empty";
        return false;
    }

    const auto icons = mdo.value("icons"_L1).toObject();
    cfg->tmplIcons.reserve(icons.size());
    for (auto i = icons.constBegin(), end = icons.constEnd(); i != end; ++i) {
        cfg->tmplIcons.insert(i.key(), i.value().toString());
    }

    const QString _statPlugin =
        core.value(QStringLiteral(HBNBOTA_CONF_CORE_STATICPLUGIN), QStringLiteral(HBNBOTA_CONF_CORE_STATICPLUGIN_DEFVAL))
            .toString();
    if (_statPlugin.compare("none"_L1, Qt::CaseInsensitive) == 0) {
        cfg->staticPlugin = Settings::StaticPlugin::None;
    } else if (_statPlugin.compare("simple"_L1, Qt::CaseInsensitive) == 0) {
        cfg->staticPlugin = Settings::StaticPlugin::Simple;
    } else if (_statPlugin.compare("compressed"_L1, Qt::CaseInsensitive) == 0) {
        cfg->staticPlugin = Settings::StaticPlugin::Compressed;
    } else {
        qCWarning(HBNBOTA_SETTINGS) << "Invalid value for" << HBNBOTA_CONF_CORE_STATICPLUGIN << "in section"
                                    << HBNBOTA_CONF_CORE
                                    << ", using default value:" << HBNBOTA_CONF_CORE_STATICPLUGIN_DEFVAL;
    }

    const QString _cache =
        core.value(QStringLiteral(HBNBOTA_CONF_CORE_CACHE), QStringLiteral(HBNBOTA_CONF_CORE_CACHE_DEFVAL)).toString();
    if (_cache.compare("none"_L1, Qt::CaseInsensitive) == 0) {
        cfg->cache = Settings::Cache::None;
    } else if (_cache.compare("memcached"_L1, Qt::CaseInsensitive) == 0) {
        cfg->cache = Settings::Cache::Memcached;
    } else {
        qCWarning(HBNBOTA_SETTINGS) << "Invalid value for" << HBNBOTA_CONF_CORE_CACHE << "in section" << HBNBOTA_CONF_CORE
                                    << ", using, default value:" << HBNBOTA_CONF_CORE_CACHE_DEFVAL;
    }

    const QString _sessionStore =
        core.value(QStringLiteral(HBNBOTA_CONF_CORE_SESSIONSTORE), QStringLiteral(HBNBOTA_CONF_CORE_SESSIONSTORE_DEFVAL))
            .toString();
    if (_sessionStore.compare("file"_L1, Qt::CaseInsensitive) == 0) {
        cfg->sessionStore = Settings::SessionStore::File;
    } else if (_sessionStore.compare("memcached"_L1, Qt::CaseInsensitive) == 0) {
        cfg->sessionStore = Settings::SessionStore::Memcached;
    } else {
        qCWarning(HBNBOTA_SETTINGS) << "Invalid value for" << HBNBOTA_CONF_CORE_SESSIONSTORE << "in section"
                                    << HBNBOTA_CONF_CORE << ", using default:" << HBNBOTA_CONF_CORE_SESSIONSTORE_DEFVAL;
    }

    return true;
}

bool loadDefaults(const QVariantMap &defaults)
{
    if (!cfg->localesLoaded) {
        qCCritical(HBNBOTA_SETTINGS) << "Can not load defaults while supported languages are not loaded";
        return false;
    }

    const QTimeZone tz{
        defaults.value(QStringLiteral(HBNBOTA_CONF_DEFAULTS_TZ), QStringLiteral(HBNBOTA_CONF_DEFAULTS_TZ_DEFVAL))
            .toString()
            .toLatin1()};
    if (tz.isValid()) {
        cfg->defTimeZone = tz;
    } else {
        qCWarning(HBNBOTA_SETTINGS) << "Invalid value for" << HBNBOTA_CONF_DEFAULTS_TZ << "in section"
                                    << HBNBOTA_CONF_DEFAULTS << ", using default value:" << HBNBOTA_CONF_DEFAULTS_TZ_DEFVAL;
    }

    QLocale defLocale{
        defaults.value(QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG), QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL))
            .toString()};
    if (defLocale == QLocale::c()) {
        qCWarning(HBNBOTA_SETTINGS) << "Invalid value for" << HBNBOTA_CONF_DEFAULTS_LANG << "in section"
                                    << HBNBOTA_CONF_DEFAULTS
                                    << ", using default value:" << HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL;
        defLocale = QLocale{QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL)};
    }

    bool foundLocale = false;
    for (const QLocale &l : std::as_const(cfg->supportedLocales)) {
        if (defLocale == l) {
            foundLocale = true;
            break;
        }
    }
    if (foundLocale) {
        cfg->defLocale = defLocale;
    } else {
        qCWarning(HBNBOTA_SETTINGS) << defLocale << "set to" << HBNBOTA_CONF_DEFAULTS_LANG << "in section"
                                    << HBNBOTA_CONF_DEFAULTS
                                    << "is not supported, using default value:" << HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL;
        cfg->defLocale = QLocale{QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL)};
    }

    return true;
}

bool Settings::load(const QVariantMap &core, const QVariantMap &defaults)
{
    QWriteLocker locker(&cfg->lock);

    if (cfg->loaded) {
        return true;
    }

    qCDebug(HBNBOTA_SETTINGS) << "Loading settings";

    if (!loadCore(core)) {
        return false;
    }

    if (!loadDefaults(defaults)) {
        return false;
    }

    cfg->loaded = true;

    return true;
}

void Settings::loadSupportedLocales(const QVector<QLocale> &locales)
{
    QWriteLocker locker(&cfg->lock);

    if (cfg->localesLoaded) {
        return;
    }

    qCDebug(HBNBOTA_SETTINGS) << "Loading supported locales";

    cfg->supportedLocales = locales;
    cfg->localesLoaded    = true;
}

QString Settings::setupToken()
{
    QReadLocker locker(&cfg->lock);
    return cfg->setupToken;
}

QString Settings::tmpl()
{
    QReadLocker locker(&cfg->lock);
    return cfg->tmpl;
}

QString Settings::tmplPath()
{
    QReadLocker locker(&cfg->lock);
    return cfg->tmplDir + '/'_L1 + cfg->tmpl;
}

QString Settings::tmplPath(QStringView path)
{
    QReadLocker locker(&cfg->lock);
    return cfg->tmplDir + '/'_L1 + cfg->tmpl + '/'_L1 + path;
}

QString Settings::tmplIcon(const QString &name)
{
    QReadLocker locker(&cfg->lock);
    return cfg->tmplIcons.value(name);
}

QString Settings::siteName()
{
    return u"Botaskaf"_s;
}

Settings::StaticPlugin Settings::staticPlugin()
{
    QReadLocker locker(&cfg->lock);
    return cfg->staticPlugin;
}

Settings::Cache Settings::cache()
{
    QReadLocker locker(&cfg->lock);
    return cfg->cache;
}

Settings::SessionStore Settings::sessionStore()
{
    QReadLocker locker(&cfg->lock);
    return cfg->sessionStore;
}

QLocale Settings::defLocale()
{
    QReadLocker locker(&cfg->lock);
    return cfg->defLocale;
}

QTimeZone Settings::defTimeZone()
{
    QReadLocker locker(&cfg->lock);
    return cfg->defTimeZone;
}

QList<CutelystForms::Option *> Settings::supportedLocales(Cutelyst::Context *c, const QLocale &selected, QObject *parent)
{
    QList<CutelystForms::Option *> lst;
    QReadLocker locker(&cfg->lock);
    lst.reserve(cfg->supportedLocales.size());
    for (const QLocale &l : std::as_const(cfg->supportedLocales)) {
        lst << new CutelystForms::Option(
            //: locale option text, %1 will be replaced by the native language name,
            //: %2 will be replaced by the native territory name, and %3 will be replaced
            //: by the locale’s short name in the form language_territory
            //% "%1 (%2) (%3)"
            c->qtTrId("hbnbota_locale_option_text").arg(l.nativeLanguageName(), l.nativeTerritoryName(), l.name()),
            l.name(),
            l == selected,
            parent);
    }

    return lst;
}

QStringList Settings::allowedLocaleIds()
{
    QStringList lst;
    QReadLocker locker(&cfg->lock);
    lst.reserve(cfg->supportedLocales.size());
    for (const QLocale &l : std::as_const(cfg->supportedLocales)) {
        lst << l.name();
    }
    return lst;
}

QList<CutelystForms::Option *> Settings::supportedTimeZones(const QByteArray &selected, QObject *parent)
{
    QList<CutelystForms::Option *> lst;
    const QList<QByteArray> tzIds = QTimeZone::availableTimeZoneIds();
    lst.reserve(tzIds.size());
    for (const QByteArray &tzId : tzIds) {
        const QString tzIdStr = QString::fromLatin1(tzId);
        lst << new CutelystForms::Option(tzIdStr, tzIdStr, tzId == selected, parent);
    }
    return lst;
}

QStringList Settings::allowedTimeZoneIds()
{
    QStringList lst;
    const QList<QByteArray> tzIds = QTimeZone::availableTimeZoneIds();
    lst.reserve(tzIds.size());
    for (const QByteArray &id : tzIds) {
        lst << QString::fromLatin1(id);
    }
    return lst;
}

QList<CutelystForms::Option *> Settings::supportedSmtpAuthMethods(Cutelyst::Context *c, const QString &selected)
{
    QList<CutelystForms::Option *> lst;
    lst.reserve(4);
    //: SMTP authentication method option name for no authentication
    //% "None"
    lst << new CutelystForms::Option(u"NONE"_s, c->qtTrId("hbnbota_smtp_authmtehod_none"), selected == "none"_L1);
    lst << new CutelystForms::Option(u"PLAIN"_s, u"PLAIN"_s, selected == "PLAIN"_L1);
    lst << new CutelystForms::Option(u"LOGIN"_s, u"LOGIN"_s, selected == "LOGIN"_L1);
    lst << new CutelystForms::Option(u"CRAM-MD5"_s, u"CRAM-MD5"_s, selected == "CRAM-MD5"_L1);
    return lst;
}

QStringList Settings::allowedSmtpAuthMethods()
{
    return {u"NONE"_s, u"PLAIN"_s, u"LOGIN"_s, u"CRAM-MD5"_s};
}
