/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "botaskaf.h"

#include "confignames.h"
#include "controllers/forms.h"
#include "controllers/login.h"
#include "controllers/logout.h"
#include "controllers/mysettings.h"
#include "controllers/root.h"
#include "controllers/setup.h"
#include "controllers/users.h"
#include "cutelee/botaskafcutelee.h"
#include "logging.h"
#include "migrations/m0001_create_users_table.h"
#include "migrations/m0002_createformstable.h"
#include "settings.h"
#include "userauthstoresql.h"

#include <Cutelyst/Engine>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationrealm.h>
#include <Cutelyst/Plugins/CSRFProtection/CSRFProtection>
#include <Cutelyst/Plugins/Memcached/Memcached>
#include <Cutelyst/Plugins/MemcachedSessionStore/MemcachedSessionStore>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Session/sessionstorefile.h>
#include <Cutelyst/Plugins/StaticCompressed/StaticCompressed>
#include <Cutelyst/Plugins/StaticSimple/StaticSimple>
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Utils/LangSelect>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Utils/Validator>
#include <Cutelyst/Plugins/View/Cutelee/cuteleeview.h>
#include <CutelystBotan/credentialbotan.h>
#include <CutelystForms/forms.h>
#include <Firfuorida/Migrator>
#include <cutelee/engine.h>

#include <QCoreApplication>
#include <QLockFile>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#if defined(QT_DEBUG)
Q_LOGGING_CATEGORY(HBNBOTA_CORE, "hbnbota.core")
#else
Q_LOGGING_CATEGORY(HBNBOTA_CORE, "hbnbota.core", QtInfoMsg)
#endif

using namespace Qt::Literals::StringLiterals;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static QMutex mutex; // clazy:exclude=non-pod-global-static

Botaskaf::Botaskaf(QObject *parent)
    : Application(parent)
{
    QCoreApplication::setApplicationName(u"Botaskaf"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(HBNBOTA_VERSION));
}

bool Botaskaf::init()
{
    const auto supportedLocales = loadTranslationsFromDir(u"botaskaf"_s, QStringLiteral(HBNBOTA_TRANSLATIONSDIR));

    Settings::loadSupportedLocales(supportedLocales);

    if (Q_UNLIKELY(!Settings::load(engine()->config(QStringLiteral(HBNBOTA_CONF_CORE)),
                                   engine()->config(QStringLiteral(HBNBOTA_CONF_DEFAULTS))))) {
        return false;
    }

    QLockFile dbInitLock{QStandardPaths::writableLocation(QStandardPaths::TempLocation) + u"/botaskaf_db.lock"_s};
    if (dbInitLock.tryLock(std::chrono::milliseconds{1})) {
        if (Q_LIKELY(connectDb(u"db"_s))) {
            if (Q_UNLIKELY(!initializeDb(u"db"_s))) {
                dbInitLock.unlock();
                return false;
            }
            QSqlDatabase::removeDatabase(u"db"_s);
        }
        dbInitLock.unlock();
    }

#if defined(QT_DEBUG)
    constexpr bool viewCache = false;
#else
    constexpr bool viewCache = true;
#endif
    qCDebug(HBNBOTA_CORE) << "View cache enabled:" << viewCache;

    auto view = new CuteleeView(this);
    view->setCache(viewCache);
    view->setWrapper(u"wrapper.html"_s);
    view->setIncludePaths({CutelystForms::Forms::templateDirPath(u"cutelee/bootstrap5"), Settings::tmplPath(u"site"_s)});
    view->engine()->addDefaultLibrary(u"cutelee_i18ntags"_s);
    view->engine()->insertDefaultLibrary(u"cutelee_botaskaf"_s, new BotaskafCutelee(view->engine()));
    qCDebug(HBNBOTA_CORE) << "Template include paths:" << view->includePaths();

    if (!Settings::setupToken().isEmpty()) {
        new Setup(this);
    } else {
        new Root(this);
        new Login(this);
        new Logout(this);
        new Users(this);
        new MySettings(this);
        new Forms(this);
    }

    const auto statPluginType = Settings::staticPlugin();
    qCDebug(HBNBOTA_CORE) << "Static plugin:" << statPluginType;
    if (statPluginType != Settings::StaticPlugin::None) {
        const QStringList staticIncPaths{Settings::tmplPath(u"static"_s)};
        qCDebug(HBNBOTA_CORE) << "Static include paths:" << staticIncPaths;
        if (statPluginType == Settings::StaticPlugin::Simple) {
            auto stat = new StaticSimple(this); // NOLINT(cppcoreguidelines-owning-memory)
            stat->setIncludePaths(staticIncPaths);
            stat->setDirs({u"css"_s, u"js"_s, u"fonts"_s});
            stat->setServeDirsOnly(true);
        } else if (statPluginType == Settings::StaticPlugin::Compressed) {
            auto stat = new StaticCompressed(this); // NOLINT(cppcoreguidelines-owning-memory)
            stat->setIncludePaths(staticIncPaths);
            stat->setDirs({u"css"_s, u"js"_s, u"fonts"_s});
            stat->setServeDirsOnly(true);
        }
    }

    const auto cacheType        = Settings::cache();
    const auto sessionStoreType = Settings::sessionStore();

    qCDebug(HBNBOTA_CORE) << "Cache:" << cacheType;
    if (cacheType == Settings::Cache::Memcached || sessionStoreType == Settings::SessionStore::Memcached) {
        auto memc = new Memcached(this); // NOLINT(cppcoreguidelines-owning-memory)
        memc->setDefaultConfig({{u"binary_protocols"_s, true}});
    }

    auto sess = new Session(this); // NOLINT(cppcoreguidelines-owning-memory)

    qCDebug(HBNBOTA_CORE) << "Session store:" << sessionStoreType;
    if (sessionStoreType == Settings::SessionStore::Memcached) {
        sess->setStorage(std::make_unique<MemcachedSessionStore>(this, sess));
    } else {
        sess->setStorage(std::make_unique<SessionStoreFile>(sess));
    }

    auto lsp = new LangSelect(this, LangSelect::Session); // NOLINT(cppcoreguidelines-owning-memory)
    lsp->setFallbackLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    lsp->setSupportedLocales(supportedLocales);
    lsp->setSessionKey(u"locale"_s);

    new CSRFProtection(this);

    new StatusMessage(this);

    auto forms = new CutelystForms::Forms(this); // NOLINT(cppcoreguidelines-owning-memory)
    forms->setIncludePaths({QStringLiteral(HBNBOTA_FORMSDIR)});

    auto authn = new Authentication(this); // NOLINT(cppcoreguidelines-owning-memory)
    authn->addRealm(std::make_shared<UserAuthStoreSql>(), std::make_shared<CutelystBotan::CredentialBotan>());

    Validator::loadTranslations(this);

    return true;
}

bool Botaskaf::postFork()
{
    QMutexLocker locker(&mutex);

    return connectDb();
}

bool Botaskaf::connectDb(const QString &conName) const
{
    const QString dbConName = conName.isEmpty() ? Sql::databaseNameThread() : conName;
    qCDebug(HBNBOTA_CORE) << "Establishing database connection" << dbConName;

    const auto conf = engine()->config(QStringLiteral(HBNBOTA_CONF_DB));
    const auto type =
        conf.value(QStringLiteral(HBNBOTA_CONF_DB_TYPE), QStringLiteral(HBNBOTA_CONF_DB_TYPE_DEFVAL)).toString().toUpper();

    QSqlDatabase db = QSqlDatabase::addDatabase(type, dbConName);
    if (Q_UNLIKELY(!db.isValid())) {
        qCCritical(HBNBOTA_CORE) << "Can not establish database connection: failed to obtain "
                                    "database object. Check your settings.";
        return false;
    }

    const auto name =
        conf.value(QStringLiteral(HBNBOTA_CONF_DB_NAME), QLatin1String(HBNBOTA_CONF_DB_NAME_DEFVAL)).toString();

    if (type == "QMYSQL"_L1 || type == "QMARIADB"_L1) {

        const auto user =
            conf.value(QStringLiteral(HBNBOTA_CONF_DB_USER), QStringLiteral(HBNBOTA_CONF_DB_USER_DEFVAL)).toString();
        const auto pass = conf.value(QStringLiteral(HBNBOTA_CONF_DB_PASS)).toString();
        const auto host =
            conf.value(QStringLiteral(HBNBOTA_CONF_DB_HOST), QStringLiteral(HBNBOTA_CONF_DB_HOST_DEFVAL)).toString();
        const auto port = conf.value(QStringLiteral(HBNBOTA_CONF_DB_PORT), HBNBOTA_CONF_DB_PORT_DEFVAL).toInt();

        db.setDatabaseName(name);
        db.setUserName(user);
        db.setPassword(pass);

        if (host[0] == '/'_L1) {
            db.setConnectOptions(u"UNIX_SOCKET=%1;MYSQL_OPT_RECONNECT=1;CLIENT_INTERACTIVE=1"_s.arg(host));
        } else {
            db.setConnectOptions(u"MYSQL_OPT_RECONNECT=1;CLIENT_INTERACTIVE=1"_s);
            db.setHostName(host);
            db.setPort(port);
        }

    } else if (type == "QSQLITE"_L1) {
        db.setDatabaseName(
            name.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + u"/botaskaf.sqlite"_s : name);
    }

    if (Q_UNLIKELY(!db.open())) {
        qCDebug(HBNBOTA_CORE) << "DB FILE:" << db.databaseName();
        qCCritical(HBNBOTA_CORE) << "Can not establish database connection:" << db.lastError().text();
        return false;
    }

    if (type == "QMYSQL"_L1 || type == "QMARIADB"_L1) {
        QSqlQuery q(db);
        if (Q_UNLIKELY(!q.exec(u"SET time_zone = '+00:00'"_s))) {
            qCWarning(HBNBOTA_CORE) << "Failed to set database connection time zone to UTC:" << q.lastError().text();
        }
    }

    if (type == "QSQLITE"_L1) {
        QSqlQuery q(db);
        if (Q_UNLIKELY(!q.exec(u"PRAGMA journal_mode = WAL"_s))) {
            qCWarning(HBNBOTA_CORE) << "Failed to set SQLite journal mode to WAL:" << q.lastError().text();
        }
        if (Q_UNLIKELY(!q.exec(u"PRAGMA encoding = 'UTF-8'"_s))) {
            qCWarning(HBNBOTA_CORE) << "Failed to set SQLite encoding mode to UTF-8:" << q.lastError().text();
        }
        if (Q_UNLIKELY(!q.exec(u"PRAGMA foreign_keys = on"_s))) {
            qCWarning(HBNBOTA_CORE) << "Failed to enable foreign keys on SQLite:" << q.lastError().text();
        }
    }

    return true;
}

bool Botaskaf::initializeDb(const QString &conName) const
{
    Firfuorida::Migrator mig{conName, u"migrations"_s};
    new M0001_CreateUsersTable(&mig);
    new M0002_CreateFormsTable(&mig);

    const QByteArray mode = qgetenv("HBNBOTA_DB_MIGRATION").toLower();

    if (mode.startsWith("refresh"_ba)) {
        auto parts = mode.split('=');
        bool ok    = true;
        uint steps = parts.size() > 1 ? parts.at(1).toUInt(&ok) : 0;
        if (Q_UNLIKELY(!ok)) {
            qCCritical(HBNBOTA_CORE) << "Invalid number of steps given for database refresh";
            return false;
        }
        if (Q_UNLIKELY(!mig.refresh(steps))) {
            qCCritical(HBNBOTA_CORE) << mig.lastError().text();
            return false;
        }
    } else if (mode.startsWith("rollback"_ba)) {
        auto parts = mode.split('=');
        bool ok    = true;
        uint steps = parts.size() > 1 ? parts.at(1).toUInt(&ok) : 1;
        if (Q_UNLIKELY(!ok)) {
            qCCritical(HBNBOTA_CORE) << "Invalid number of steps given for database rollback";
            return false;
        }
        if (Q_UNLIKELY(!mig.rollback(steps))) {
            qCCritical(HBNBOTA_CORE) << mig.lastError().text();
            return false;
        }
    } else if (mode == "reset"_ba) {
        if (Q_UNLIKELY(!mig.reset())) {
            qCCritical(HBNBOTA_CORE) << mig.lastError().text();
            return false;
        }
    } else {
        if (Q_UNLIKELY(!mig.migrate())) {
            qCCritical(HBNBOTA_CORE) << mig.lastError().text();
            return false;
        }
    }

    return true;
}

#include "moc_botaskaf.cpp"
