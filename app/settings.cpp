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

    bool loaded{false};
};

Q_GLOBAL_STATIC(SettingVals, cfg) // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

bool Settings::load()
{
    QWriteLocker locker(&cfg->lock);

    if (cfg->loaded) {
        return true;
    }

    qCDebug(HBNBOTA_SETTINGS) << "Loading settings";

    cfg->loaded = true;

    return true;
}
