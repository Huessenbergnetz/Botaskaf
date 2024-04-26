/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "settings.h"

#include "confignames.h"
#include "logging.h"

#include <Cutelyst/Context>

#include <QGlobalStatic>
#include <QReadLocker>
#include <QReadWriteLock>
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
    QString tmpl;

    QLocale defLocale{QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL)};

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
    cfg->tmpl       = core.value(QStringLiteral(HBNBOTA_CONF_CORE_TEMPLATE),
                           QStringLiteral(HBNBOTA_CONF_CORE_TEMPLATE_DEFVAL))
                    .toString();

    const QString _statPlugin = core.value(QStringLiteral(HBNBOTA_CONF_CORE_STATICPLUGIN),
                                           QStringLiteral(HBNBOTA_CONF_CORE_STATICPLUGIN_DEFVAL))
                                    .toString();
    if (_statPlugin.compare("none"_L1, Qt::CaseInsensitive) == 0) {
        cfg->staticPlugin = Settings::StaticPlugin::None;
    } else if (_statPlugin.compare("simple"_L1, Qt::CaseInsensitive) == 0) {
        cfg->staticPlugin = Settings::StaticPlugin::Simple;
    } else if (_statPlugin.compare("compressed"_L1, Qt::CaseInsensitive) == 0) {
        cfg->staticPlugin = Settings::StaticPlugin::Compressed;
    } else {
        qCWarning(HBNBOTA_SETTINGS)
            << "Invalid value for" << HBNBOTA_CONF_CORE_STATICPLUGIN << "in section"
            << HBNBOTA_CONF_CORE
            << ", using default value:" << HBNBOTA_CONF_CORE_STATICPLUGIN_DEFVAL;
    }

    const QString _cache = core.value(QStringLiteral(HBNBOTA_CONF_CORE_CACHE),
                                      QStringLiteral(HBNBOTA_CONF_CORE_CACHE_DEFVAL))
                               .toString();
    if (_cache.compare("none"_L1, Qt::CaseInsensitive) == 0) {
        cfg->cache = Settings::Cache::None;
    } else if (_cache.compare("memcached"_L1, Qt::CaseInsensitive) == 0) {
        cfg->cache = Settings::Cache::Memcached;
    } else {
        qCWarning(HBNBOTA_SETTINGS)
            << "Invalid value for" << HBNBOTA_CONF_CORE_CACHE << "in section" << HBNBOTA_CONF_CORE
            << ", using, default value:" << HBNBOTA_CONF_CORE_CACHE_DEFVAL;
    }

    const QString _sessionStore = core.value(QStringLiteral(HBNBOTA_CONF_CORE_SESSIONSTORE),
                                             QStringLiteral(HBNBOTA_CONF_CORE_SESSIONSTORE_DEFVAL))
                                      .toString();
    if (_sessionStore.compare("file"_L1, Qt::CaseInsensitive) == 0) {
        cfg->sessionStore = Settings::SessionStore::File;
    } else if (_sessionStore.compare("memcached"_L1, Qt::CaseInsensitive) == 0) {
        cfg->sessionStore = Settings::SessionStore::Memcached;
    } else {
        qCWarning(HBNBOTA_SETTINGS)
            << "Invalid value for" << HBNBOTA_CONF_CORE_SESSIONSTORE << "in section"
            << HBNBOTA_CONF_CORE << ", using default:" << HBNBOTA_CONF_CORE_SESSIONSTORE_DEFVAL;
    }

    return true;
}

bool loadDefaults(const QVariantMap &defaults)
{
    if (!cfg->localesLoaded) {
        qCCritical(HBNBOTA_SETTINGS)
            << "Can not load defaults while supported languages are not loaded";
        return false;
    }

    QLocale defLocale{defaults
                          .value(QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG),
                                 QStringLiteral(HBNBOTA_CONF_DEFAULTS_LANG_DEFVAL))
                          .toString()};
    if (defLocale == QLocale::c()) {
        qCWarning(HBNBOTA_SETTINGS)
            << "Invalid value for" << HBNBOTA_CONF_DEFAULTS_LANG << "in section"
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
        qCWarning(HBNBOTA_SETTINGS)
            << defLocale << "set to" << HBNBOTA_CONF_DEFAULTS_LANG << "in section"
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
    return QStringLiteral(HBNBOTA_TEMPLATESDIR) + '/'_L1 + cfg->tmpl;
}

QString Settings::tmplPath(QStringView path)
{
    QReadLocker locker(&cfg->lock);
    return QStringLiteral(HBNBOTA_TEMPLATESDIR) + '/'_L1 + cfg->tmpl + '/'_L1 + path;
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

QList<CutelystForms::Option *>
    Settings::supportedLocales(Cutelyst::Context *c, const QLocale &selected, QObject *parent)
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
            c->qtTrId("hbnbota_locale_option_text")
                .arg(l.nativeLanguageName(), l.nativeTerritoryName(), l.name()),
            l.name(),
            l == selected,
            parent);
    }

    return lst;
}
