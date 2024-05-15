/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "user.h"

#include "error.h"
#include "logging.h"
#include "settings.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Memcached/memcached.h>
#include <Cutelyst/Plugins/Utils/sql.h>
#include <CutelystBotan/credentialbotan.h>

#include <QDebug>
#include <QJsonDocument>
#include <QMetaEnum>
#include <QMetaObject>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

using namespace Qt::Literals::StringLiterals;

#define HBNBOTA_USER_STASH_KEY u"user"_s
#define HBNBOTA_USER_MEMC_GROUP_KEY "users"_ba

class UserData : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
    UserData() noexcept = default;
    UserData(User::dbid_t _id,
             User::Type _type,
             const QString &_email,
             const QString &_displayName,
             const QDateTime &_created,
             const QDateTime &_updated,
             const QDateTime &_lastSeen,
             const QDateTime &_lockedAt,
             User::dbid_t _lockedById,
             const QString &_lockedByName,
             const QVariantMap &_settings)
        : QSharedData()
        , email{_email}
        , displayName{_displayName}
        , lockedByName{_lockedByName}
        , created{_created}
        , updated{_updated}
        , lastSeen{_lastSeen}
        , lockedAt{_lockedAt}
        , settings{_settings}
        , id{_id}
        , lockedById{_lockedById}
        , type{_type}
    {
        created.setTimeSpec(Qt::UTC);
        updated.setTimeSpec(Qt::UTC);
        if (lastSeen.isValid()) {
            lastSeen.setTimeSpec(Qt::UTC);
        }
        if (lockedAt.isValid()) {
            lockedAt.setTimeSpec(Qt::UTC);
        }
    }

    UserData(const UserData &) noexcept   = default;
    UserData &operator=(const UserData &) = delete;
    ~UserData() noexcept                  = default;

    void setUrls(Cutelyst::Context *c);

    QString email;
    QString displayName;
    QString lockedByName;
    QDateTime created;
    QDateTime updated;
    QDateTime lastSeen;
    QDateTime lockedAt;
    QVariantMap settings;
    QUrl editUrl;
    User::dbid_t id{0};
    User::dbid_t lockedById{0};
    User::Type type{User::Invalid};
};

void UserData::setUrls(Cutelyst::Context *c)
{
    User current = User::fromStash(c);
    if (id == current.id()) {
        editUrl = c->uriFor(c->getAction(u"index", u"mysettings"));
    } else if (current.isAdmin()) {
        editUrl = c->uriFor(c->getAction(u"edit", u"users"), {}, {QString::number(id)});
    }
}

User::User() noexcept = default;

User::User(dbid_t id,
           Type type,
           const QString &email,
           const QString &displayName,
           const QDateTime &created,
           const QDateTime &updated,
           const QDateTime &lastSeen,
           const QDateTime &lockedAt,
           dbid_t lockedById,
           const QString &lockedByName,
           const QVariantMap &settings)
    : data{new UserData{id,
                        type,
                        email,
                        displayName,
                        created,
                        updated,
                        lastSeen,
                        lockedAt,
                        lockedById,
                        lockedByName,
                        settings}}
{
}

User::User(const User &user) noexcept = default;

User::User(User &&user) noexcept = default;

User &User::operator=(const User &user) noexcept = default;

User &User::operator=(User &&user) noexcept = default;

User::~User() noexcept = default;

User::dbid_t User::id() const noexcept
{
    return data ? data->id : 0;
}

User::Type User::type() const noexcept
{
    return data ? data->type : Invalid;
}

QString User::email() const noexcept
{
    return data ? data->email : QString();
}

QString User::displayName() const noexcept
{
    return data ? data->displayName : QString();
}

QDateTime User::created() const noexcept
{
    return data ? data->created : QDateTime();
}

QDateTime User::updated() const noexcept
{
    return data ? data->updated : QDateTime();
}

QDateTime User::lastSeen() const noexcept
{
    return data ? data->lastSeen : QDateTime();
}

QDateTime User::lockedAt() const noexcept
{
    return data ? data->lockedAt : QDateTime();
}

User::dbid_t User::lockedById() const noexcept
{
    return data ? data->lockedById : 0;
}

QString User::lockedByName() const noexcept
{
    return data ? data->lockedByName : QString();
}

QVariantMap User::settings() const
{
    return data ? data->settings : QVariantMap();
}

QVariant User::setting(const QString &key, const QVariant &defValue) const
{
    return data ? data->settings.value(key, defValue) : QVariant();
}

QString User::timezone() const
{
    return setting(u"timezone"_s, u"UTC"_s).toString();
}

QString User::locale() const
{
    return setting(u"locale"_s, u"en_US"_s).toString();
}

QUrl User::editUrl() const noexcept
{
    return data ? data->editUrl : QUrl();
}

bool User::isAdmin() const noexcept
{
    return data && data->type >= Administrator;
}

bool User::isValid() const noexcept
{
    return data && data->id > 0 && data->type != Invalid;
}

QJsonObject User::toJson() const
{
    if (isNull() || !isValid()) {
        return {};
    }

    return {{u"id"_s, static_cast<qint64>(data->id)},
            {u"type"_s, static_cast<int>(data->type)},
            {u"email"_s, data->email},
            {u"displayName"_s, data->displayName},
            {u"created"_s, data->created.toString(Qt::ISODateWithMs)},
            {u"updated"_s, data->updated.toString(Qt::ISODateWithMs)},
            {u"lastSeen"_s, data->lastSeen.isNull() ? QJsonValue() : data->lastSeen.toString(Qt::ISODateWithMs)},
            {u"lockedAt"_s, data->lockedAt.isNull() ? QJsonValue() : data->lockedAt.toString(Qt::ISODateWithMs)},
            {u"lockedById"_s, static_cast<qint64>(data->lockedById)},
            {u"lockedByName"_s, data->lockedByName.isEmpty() ? QJsonValue() : data->lockedByName},
            {u"settings"_s, QJsonObject::fromVariantMap(data->settings)},
            {u"isAdmin"_s, isAdmin()}};
}

bool User::operator==(const User &other) const noexcept
{
    if (data == other.data) {
        return true;
    }

    if (id() != other.id()) {
        return false;
    }

    if (type() != other.type()) {
        return false;
    }

    if (email() != other.email()) {
        return false;
    }

    if (displayName() != other.displayName()) {
        return false;
    }

    if (created() != other.created()) {
        return false;
    }

    if (updated() != other.updated()) {
        return false;
    }

    if (lastSeen() != other.lastSeen()) {
        return false;
    }

    if (lockedAt() != other.lockedAt()) {
        return false;
    }

    if (lockedById() != other.lockedById()) {
        return false;
    }

    if (lockedByName() != other.lockedByName()) {
        return false;
    }

    if (settings() != other.settings()) {
        return false;
    }

    return true;
}

User::Type User::typeStringToEnum(QStringView str)
{
    if (str.compare(u"disabled", Qt::CaseInsensitive) == 0) {
        return User::Disabled;
    } else if (str.compare(u"registered", Qt::CaseInsensitive) == 0) {
        return User::Registered;
    } else if (str.compare(u"manager", Qt::CaseInsensitive) == 0) {
        return User::Manager;
    } else if (str.compare(u"administrator", Qt::CaseInsensitive) == 0) {
        return User::Administrator;
    } else if (str.compare(u"superuser", Qt::CaseInsensitive) == 0) {
        return User::SuperUser;
    } else {
        return User::Invalid;
    }
}

QString User::typeEnumToString(User::Type type)
{
    if (type != User::Invalid) {
        const auto mo = User::staticMetaObject;
        const auto me = mo.enumerator(mo.indexOfEnumerator("Type"));

        return QString::fromLocal8Bit(me.valueToKey(static_cast<int>(type)));
    }

    return {};
}

QList<CutelystForms::Option *> User::supportedTypes(User::Type selected)
{
    QList<CutelystForms::Option *> lst;
    const auto selectedInt = static_cast<int>(selected);

    const auto mo = User::staticMetaObject;
    const auto me = mo.enumerator(mo.indexOfEnumerator("Type"));

    lst.reserve(me.keyCount() - 1);

    for (int i = 1; i < me.keyCount(); ++i) {
        const auto val = me.value(i);
        lst << new CutelystForms::Option(QString::fromLocal8Bit(me.key(i)), QString::number(val), val == selectedInt);
    }

    return lst;
}

QList<CutelystForms::Option *> User::supportedTypesBelow(Type below, Type selected)
{
    QList<CutelystForms::Option *> lst;
    const auto selectedInt = static_cast<int>(selected);
    const auto belowInt    = static_cast<int>(below);

    const auto mo = User::staticMetaObject;
    const auto me = mo.enumerator(mo.indexOfEnumerator("Type"));

    for (int i = 1; i < me.keyCount(); ++i) {
        const auto val = me.value(i);
        if (val < belowInt) {
            lst << new CutelystForms::Option(QString::fromLocal8Bit(me.key(i)), QString::number(val), val == selectedInt);
        }
    }

    return lst;
}

QStringList User::typeValues()
{
    QStringList lst;

    const auto mo = User::staticMetaObject;
    const auto me = mo.enumerator(mo.indexOfEnumerator("Type"));

    lst.reserve(me.keyCount() - 1);

    for (int i = 1; i < me.keyCount(); ++i) {
        lst << QString::number(me.value(i));
    }

    return lst;
}

QStringList User::typeValues(User::Type below)
{
    QStringList lst;
    const int belowInt = static_cast<int>(below);

    const auto mo = User::staticMetaObject;
    const auto me = mo.enumerator(mo.indexOfEnumerator("Type"));

    for (int i = 1; i < me.keyCount(); ++i) {
        const int val = me.value(i);
        if (val < belowInt) {
            lst << QString::number(val); // clazy:exclude=reserve-candidates
        }
    }

    return lst;
}

User::dbid_t User::toDbId(qulonglong id, bool *ok)
{
    if (id > static_cast<qulonglong>(std::numeric_limits<User::dbid_t>::max())) {
        if (ok) {
            *ok = false;
        }
        return 0;
    }

    if (ok) {
        *ok = true;
    }

    return static_cast<User::dbid_t>(id);
}

User::dbid_t User::toDbId(const QVariant &var, bool *ok)
{
    bool _ok      = false;
    const auto id = var.toULongLong(&_ok);

    if (_ok) {
        return toDbId(id, ok);
    }

    if (ok) {
        *ok = false;
    }

    return 0;
}

QMap<QString, QString> User::labels(Cutelyst::Context *c)
{
    return {
        //% "id"
        {u"id"_s, c->qtTrId("hbnbota_user_label_id")},
        //% "type"
        {u"type"_s, c->qtTrId("hbnbota_user_label_type")},
        //% "email"
        {u"email"_s, c->qtTrId("hbnbota_user_label_email")},
        //% "display name"
        {u"displayName"_s, c->qtTrId("hbnbota_user_label_displayname")},
        //% "created"
        {u"created"_s, c->qtTrId("hbnbota_user_label_created")},
        //% "updated"
        {u"updated"_s, c->qtTrId("hbnbota_user_label_updated")},
        //% "last seen"
        {u"lastSeen"_s, c->qtTrId("hbnbota_user_label_lastseen")},
    };
}

User User::fromStash(Cutelyst::Context *c)
{
    Q_ASSERT(c);
    return c->stash(HBNBOTA_USER_STASH_KEY).value<User>();
}

void User::toStash(Cutelyst::Context *c) const
{
    Q_ASSERT(c);
    c->setStash(HBNBOTA_USER_STASH_KEY, QVariant::fromValue<User>(*this));
}

User User::create(Cutelyst::Context *c, Error &e, const QVariantHash &values)
{
    const QString email       = values.value(u"email"_s).toString();
    const QString displayName = values.value(u"displayName"_s).toString();
    const QString password    = values.value(u"password"_s).toString();
    const Type type           = static_cast<Type>(values.value(u"type"_s).toInt());
    const QDateTime now       = QDateTime::currentDateTimeUtc();
    const QJsonDocument settings{QJsonObject{{u"locale"_s, values.value(u"locale"_s).toString()},
                                             {u"timezone"_s, values.value(u"timezone"_s).toString()}}};

    const QString passwordHash = CutelystBotan::CredentialBotan::createArgon2Password(password);
    if (Q_UNLIKELY(passwordHash.isEmpty())) {
        e = Error::create(c,
                          Cutelyst::Response::InternalServerError,
                          //% "Failed to hash the password."
                          c->qtTrId("hbnbota_error_failed_hashing_pw"));
        qCCritical(HBNBOTA_CORE) << "Failed to hash the password for new user identified by email" << email;
        return {};
    }

    QSqlQuery q =
        CPreparedSqlQueryThread(u"INSERT INTO users (type, email, displayName, password, created, updated, settings) "
                                "VALUES (:type, :email, :displayName, :password, :created, :updated, :settings)"_s);

    if (Q_UNLIKELY(q.lastError().isValid())) {
        //% "Failed to insert new user “%1” into database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_user_failed_create_db").arg(email));
        qCCritical(HBNBOTA_CORE) << "Failed to insert new user" << email << "into database:" << q.lastError().text();
        return {};
    }

    q.bindValue(u":type"_s, static_cast<int>(type));
    q.bindValue(u":email"_s, email);
    q.bindValue(u":displayName"_s, displayName);
    q.bindValue(u":password"_s, passwordHash);
    q.bindValue(u":created"_s, now);
    q.bindValue(u":updated"_s, now);
    q.bindValue(u":settings"_s, settings.toJson(QJsonDocument::Compact));

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(c, q, c->qtTrId("hbnbota_error_user_failed_create_db").arg(email));
        qCCritical(HBNBOTA_CORE) << "Failed to insert new user" << email << "into database:" << q.lastError().text();
        return {};
    }

    User::dbid_t id = 0;

    if (q.driver()->hasFeature(QSqlDriver::LastInsertId)) {
        id = User::toDbId(q.lastInsertId());
    } else {
        q = CPreparedSqlQueryThreadFO(u"SELECT id FROM users WHERE email = :email"_s);
        q.bindValue(u":email"_s, email);
        q.exec();
        q.next();
        id = User::toDbId(q.value(0));
    }

    User u{id, type, email, displayName, now, now, {}, {}, 0, {}, settings.object().toVariantMap()};
    u.toCache();

    qCInfo(HBNBOTA_CORE) << User::fromStash(c) << "created new" << u;

    return u;
}

User User::get(Cutelyst::Context *c, Error &e, User::dbid_t id)
{
    User u = User::fromCache(id);
    if (!u.isNull()) {
        u.data->setUrls(c);
        return u;
    }

    qCDebug(HBNBOTA_CORE) << "Query user with ID" << id << "from the database";

    QSqlQuery q = CPreparedSqlQueryThreadFO(
        u"SELECT u1.type, u1.email, u1.displayName, u1.created, u1.updated, u1.lastSeen, u1.lockedAt, u1.lockedBy, u2.displayName AS lockedByName, u1.settings FROM users u1 LEFT JOIN users u2 ON u2.id = u1.lockedBy WHERE u1.id = :id"_s);
    q.bindValue(u":id"_s, id);

    if (Q_UNLIKELY(!q.exec())) {
        //% "Failed to get user with ID %1 from database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_user_get_query_failed").arg(id));
        qCCritical(HBNBOTA_CORE) << "Failed to get user with ID" << id << "from database:" << q.lastError().text();
        return {};
    }

    if (Q_UNLIKELY(!q.next())) {
        //% "Can not find user with ID %1 in the database."
        e = Error::create(c, Cutelyst::Response::NotFound, c->qtTrId("hbnbota_error_user_get_not_found").arg(id));
        qCCritical(HBNBOTA_CORE) << "Can not find user ID" << id << "in the database";
        return {};
    }

    const User::Type type         = static_cast<User::Type>(q.value(0).toInt());
    const QString email           = q.value(1).toString();
    const QString displayName     = q.value(2).toString();
    const QDateTime created       = q.value(3).toDateTime();
    const QDateTime updated       = q.value(4).toDateTime();
    const QDateTime lastSeen      = q.value(5).toDateTime();
    const QDateTime lockedAt      = q.value(6).toDateTime();
    const User::dbid_t lockedById = User::toDbId(q.value(7));
    const QString lockedByName    = q.value(8).toString();
    const QVariantMap settings    = QJsonDocument::fromJson(q.value(9).toByteArray()).object().toVariantMap();

    u = User{id, type, email, displayName, created, updated, lastSeen, lockedAt, lockedById, lockedByName, settings};
    u.data->setUrls(c);
    u.toCache();

    return u;
}

QList<User> User::list(Cutelyst::Context *c, Error &e)
{
    QSqlQuery q = CPreparedSqlQueryThreadFO(
        u"SELECT u1.id, u1.type, u1.email, u1.displayName, u1.created, u1.updated, u1.lastSeen, u1.lockedAt, u1.lockedBy, u2.displayName AS lockedByName, u1.settings FROM users u1 LEFT JOIN users u2 ON u2.id = u1.lockedBy"_s);

    if (Q_UNLIKELY(q.lastError().isValid())) {
        //% "Failed to query users from database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_user_list_query_failed"));
        qCCritical(HBNBOTA_CORE) << "Failed to query users from database:" << q.lastError().text();
        return {};
    }

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(c, q, c->qtTrId("hbnbota_error_user_list_query_failed"));
        qCCritical(HBNBOTA_CORE) << "Failed to query users from database:" << q.lastError().text();
        return {};
    }

    QList<User> lst;
    if (q.size() > 0) {
        lst.reserve(q.size());
    }

    while (q.next()) {
        User u{q.value(0).toUInt(),
               static_cast<User::Type>(q.value(01).toInt()),
               q.value(2).toString(),
               q.value(3).toString(),
               q.value(4).toDateTime(),
               q.value(5).toDateTime(),
               q.value(6).toDateTime(),
               q.value(7).toDateTime(),
               q.value(8).toUInt(),
               q.value(9).toString(),
               QJsonDocument::fromJson(q.value(10).toByteArray()).object().toVariantMap()};
        u.data->setUrls(c);
        lst << u;
    }

    return lst;
}

bool User::toStash(Cutelyst::Context *c, Error &e, User::dbid_t id)
{
    Q_ASSERT(c);
    const auto u = User::get(c, e, id);
    if (!u.isNull()) {
        u.toStash(c);
        return true;
    }
    return false;
}

bool User::toStash(Cutelyst::Context *c, Error &e, const Cutelyst::AuthenticationUser &authUser)
{
    Q_ASSERT(c);
    bool ok       = false;
    const auto id = User::toDbId(authUser.id(), &ok);
    if (!ok) {
        return false;
    }
    return User::toStash(c, e, id);
}

void User::updateLastSeen(Cutelyst::Context *c)
{
    Q_UNUSED(c);

    const auto ls = QDateTime::currentDateTimeUtc();

    QSqlQuery q = CPreparedSqlQueryThread(u"UPDATE users SET lastSeen = :lastSeen WHERE id = :id"_s);

    if (Q_UNLIKELY(q.lastError().isValid())) {
        qCCritical(HBNBOTA_CORE) << "Failed to update lastSeen state of" << *this << "in database:" << q.lastError().text();
        return;
    }

    q.bindValue(u":lastSeen"_s, ls);
    q.bindValue(u":id"_s, id());

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(HBNBOTA_CORE) << "Failed to update lastSeen state of" << *this << "in database:" << q.lastError().text();
        return;
    }

    data->lastSeen = ls;

    toCache();
}

User User::fromCache(User::dbid_t id)
{
    User u;

    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::ReturnType rt{Cutelyst::Memcached::ReturnType::Failure};
        u = Cutelyst::Memcached::getByKey<User>(HBNBOTA_USER_MEMC_GROUP_KEY, QByteArray::number(id), nullptr, &rt);
        if (rt == Cutelyst::Memcached::ReturnType::Success) {
            qCDebug(HBNBOTA_CORE) << "Found user with ID" << id << "in memcached";
        }
    }

    return u;
}

void User::toCache() const
{
    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::setByKey<User>(
            HBNBOTA_USER_MEMC_GROUP_KEY, QByteArray::number(id()), *this, std::chrono::days{7});
    }
}

QDebug operator<<(QDebug dbg, const User &user)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "User(";
    if (!user.isNull()) {
        if (user.isValid()) {
            dbg << "ID: " << user.id();
            dbg << ", Email: " << user.email();
            dbg << ", Type: " << user.type();
            dbg << ", Display Name: " << user.displayName();
        } else {
            dbg << "INVALID";
        }
    }
    dbg << ')';
    return dbg;
}

QDataStream &operator<<(QDataStream &out, const User &user)
{
    if (!user.isNull()) {
        out << user.data->id << static_cast<qint32>(user.data->type) << user.data->email << user.data->displayName
            << user.data->created << user.data->updated << user.data->lastSeen << user.data->lockedAt
            << user.data->lockedById << user.data->lockedByName << user.data->settings;
    } else {
        out << static_cast<User::dbid_t>(0);
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, User &user)
{
    User::dbid_t id = 0;
    in >> id;

    if (id == 0) {
        user.clear();
    } else {
        if (user.data == nullptr) {
            user.data = new UserData; // NOLINT(cppcoreguidelines-owning-memory)
        }

        user.data->id  = id;
        qint32 typeInt = -1;
        in >> typeInt;
        user.data->type = static_cast<User::Type>(typeInt);
        in >> user.data->email;
        in >> user.data->displayName;
        in >> user.data->created;
        in >> user.data->updated;
        in >> user.data->lastSeen;
        in >> user.data->lockedAt;
        in >> user.data->lockedById;
        in >> user.data->lockedByName;
        in >> user.data->settings;
    }

    return in;
}

void swap(User &a, User &b) noexcept
{
    a.swap(b);
}

#include "moc_user.cpp"
