/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "settings.h"

#include "confignames.h"
#include "logging.h"

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

    QString setupToken;
    QString tmpl;

    bool loaded{false};
};

Q_GLOBAL_STATIC(SettingVals, cfg) // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

bool loadCore(const QVariantMap &core)
{
    cfg->setupToken = core.value(QStringLiteral(HBNBOTA_CONF_CORE_SETUPTOKEN)).toString();
    cfg->tmpl       = core.value(QStringLiteral(HBNBOTA_CONF_CORE_TEMPLATE),
                           QStringLiteral(HBNBOTA_CONF_CORE_TEMPLATE_DEFVAL))
                    .toString();

    return true;
}

bool Settings::load(const QVariantMap &core)
{
    QWriteLocker locker(&cfg->lock);

    if (cfg->loaded) {
        return true;
    }

    qCDebug(HBNBOTA_SETTINGS) << "Loading settings";

    if (!loadCore(core)) {
        return false;
    }

    cfg->loaded = true;

    return true;
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
