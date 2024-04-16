/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "user.h"

#include <QDebug>
#include <QMetaEnum>
#include <QMetaObject>

using namespace Qt::Literals::StringLiterals;

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

    QString email;
    QString displayName;
    QString lockedByName;
    QDateTime created;
    QDateTime updated;
    QDateTime lastSeen;
    QDateTime lockedAt;
    QVariantMap settings;
    User::dbid_t id{0};
    User::dbid_t lockedById{0};
    User::Type type{User::Invalid};
};

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
            {u"lastSeen"_s,
             data->lastSeen.isNull() ? QJsonValue() : data->lastSeen.toString(Qt::ISODateWithMs)},
            {u"lockedAt"_s,
             data->lockedAt.isNull() ? QJsonValue() : data->lockedAt.toString(Qt::ISODateWithMs)},
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

User::Type User::typeStringToEnum(const QString &str)
{
    if (str.compare("disabled"_L1, Qt::CaseInsensitive) == 0) {
        return User::Disabled;
    } else if (str.compare("registered"_L1, Qt::CaseInsensitive) == 0) {
        return User::Registered;
    } else if (str.compare("manager"_L1, Qt::CaseInsensitive) == 0) {
        return User::Manager;
    } else if (str.compare("administrator"_L1, Qt::CaseInsensitive) == 0) {
        return User::Administrator;
    } else if (str.compare("superuser"_L1, Qt::CaseInsensitive) == 0) {
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

QStringList User::supportedTypes()
{
    QStringList lst;

    const auto mo = User::staticMetaObject;
    const auto me = mo.enumerator(mo.indexOfEnumerator("Type"));

    lst.reserve(me.keyCount() - 1);

    for (int i = 1; i < me.keyCount(); ++i) {
        lst.append(QString::fromLocal8Bit(me.key(i)));
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

User::dbid_t User::toDbId(const QString &str, bool *ok)
{
    bool _ok      = false;
    const auto id = str.toULongLong(&_ok);

    if (_ok) {
        return toDbId(id, ok);
    }

    if (ok) {
        *ok = false;
    }

    return 0;
}

QDebug operator<<(QDebug dbg, const User &user)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "User(";
    if (!user.isNull()) {
        if (user.isValid()) {
            dbg << "DI: " << user.id();
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
        out << user.data->id << static_cast<qint32>(user.data->type) << user.data->email
            << user.data->displayName << user.data->created << user.data->updated
            << user.data->lastSeen << user.data->lockedAt << user.data->lockedById
            << user.data->lockedByName << user.data->settings;
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
