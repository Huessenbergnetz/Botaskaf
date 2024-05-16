/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "form.h"

#include "logging.h"
#include "settings.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Memcached/memcached.h>
#include <Cutelyst/Plugins/Utils/sql.h>

#include <QUuid>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;

#define HBNBOTA_FORM_STASH_KEY u"contactform"_s
#define HBNBOTA_FORM_MEMC_GROUP_KEY "forms"_ba

Form::Data::Data(Form::dbid_t _id,
                 const QString &_uuid,
                 const User &_user,
                 const QString &_secret,
                 const QString &_name,
                 const QString &_domain,
                 const QString &_description,
                 const QDateTime &_created,
                 const QDateTime &_updated,
                 const QDateTime &_lockedAt,
                 const User &_lockedBy,
                 const QVariantMap &_settings)
    : QSharedData()
    , user{_user}
    , lockedBy{_lockedBy}
    , uuid{_uuid}
    , secret{_secret}
    , name{_name}
    , domain{_domain}
    , description{_description}
    , created{_created}
    , updated{_updated}
    , lockedAt{_lockedAt}
    , settings{_settings}
    , id{_id}
{
    created.setTimeSpec(Qt::UTC);
    updated.setTimeSpec(Qt::UTC);
    if (lockedAt.isValid()) {
        lockedAt.setTimeSpec(Qt::UTC);
    }
}

Form::Form(dbid_t id,
           const QString &uuid,
           const User &user,
           const QString &secret,
           const QString &name,
           const QString &domain,
           const QString &description,
           const QDateTime &created,
           const QDateTime &updated,
           const QDateTime &lockedAt,
           const User &lockedBy,
           const QVariantMap &settings)
    : data{new Form::Data{id, uuid, user, secret, name, domain, description, created, updated, lockedAt, lockedBy, settings}}
{
}

Form::dbid_t Form::id() const noexcept
{
    return data ? data->id : 0;
}

QString Form::uuid() const noexcept
{
    return data ? data->uuid : QString();
}

User Form::user() const noexcept
{
    return data ? data->user : User();
}

QString Form::secret() const noexcept
{
    return data ? data->secret : QString();
}

QString Form::name() const noexcept
{
    return data ? data->name : QString();
}

QString Form::domain() const noexcept
{
    return data ? data->domain : QString();
}

QString Form::description() const noexcept
{
    return data ? data->description : QString();
}

QDateTime Form::created() const noexcept
{
    return data ? data->created : QDateTime();
}

QDateTime Form::updated() const noexcept
{
    return data ? data->updated : QDateTime();
}

QDateTime Form::lockedAt() const noexcept
{
    return data ? data->lockedAt : QDateTime();
}

User Form::lockedBy() const noexcept
{
    return data ? data->lockedBy : User();
}

QVariantMap Form::settings() const noexcept
{
    return data ? data->settings : QVariantMap();
}

bool Form::isValid() const noexcept
{
    return data && data->id > 0;
}

Form::dbid_t Form::toDbId(qulonglong id, bool *ok)
{
    if (id > static_cast<qulonglong>(std::numeric_limits<Form::dbid_t>::max())) {
        if (ok) {
            *ok = false;
        }
        return 0;
    }

    if (ok) {
        *ok = true;
    }

    return static_cast<Form::dbid_t>(id);
}

Form::dbid_t Form::toDbId(const QVariant &var, bool *ok)
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

Form Form::fromStash(Cutelyst::Context *c)
{
    Q_ASSERT(c);
    return c->stash(HBNBOTA_FORM_STASH_KEY).value<Form>();
}

void Form::toStash(Cutelyst::Context *c) const
{
    Q_ASSERT(c);
    c->setStash(HBNBOTA_FORM_STASH_KEY, QVariant::fromValue<Form>(*this));
}

Form Form::create(Cutelyst::Context *c, Error &e, const QVariantHash &values)
{
    const User user = User::get(c, e, User::toDbId(values.value(u"userId"_s)));
    if (!user.isValid()) {
        return {};
    }
    // const QString uuid = QUuid::createUuid().toString(QUuid::Id128).toLower();
    // const QString secret = QUuid::createUuid().toString(QUuid::Id128).toUpper();
    // const QString name = values.value(u"name"_s).toString();
    // const QString domain = values.value(u"domain"_s).toString();
    // const QString description = values.value(u"description"_s).toString();
    // const QDateTime now = QDateTime::currentDateTimeUtc();

    return {};
}

Form Form::fromCache(Form::dbid_t id)
{
    Form f;

    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::ReturnType rt{Cutelyst::Memcached::ReturnType::Failure};
        f = Cutelyst::Memcached::getByKey<Form>(HBNBOTA_FORM_MEMC_GROUP_KEY, QByteArray::number(id), nullptr, &rt);
        if (rt == Cutelyst::Memcached::ReturnType::Success) {
            qCDebug(HBNBOTA_CORE) << "Found contact form with ID" << id << "in memcached";
        }
    }

    return f;
}

void Form::toCache() const
{
    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::setByKey<Form>(
            HBNBOTA_FORM_MEMC_GROUP_KEY, QByteArray::number(id()), *this, std::chrono::days{7});
    }
}

QDebug operator<<(QDebug dbg, const Form &form)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Form(";
    if (!form.isNull()) {
        if (form.isValid()) {
            dbg << "ID: " << form.id();
            dbg << ", Name: " << form.name();
            dbg << ", Domain: " << form.domain();
            dbg << ", UUID: " << form.uuid();
        } else {
            dbg << "INVALID";
        }
    }
    dbg << ')';
    return dbg;
}

QDataStream &operator<<(QDataStream &out, const Form &form)
{
    if (!form.isNull()) {
        out << form.data->id << form.data->uuid << form.data->user << form.data->secret << form.data->name
            << form.data->domain << form.data->description << form.data->created << form.data->updated << form.data->lockedAt
            << form.data->lockedBy << form.data->settings;
    } else {
        out << static_cast<Form::dbid_t>(0);
    }

    return out;
}

QDataStream &operator>>(QDataStream &in, Form &form)
{
    Form::dbid_t id = 0;
    in >> id;

    if (id == 0) {
        form.clear();
    } else {
        if (form.data == nullptr) {
            form.data = new Form::Data;
        }

        form.data->id = id;
        in >> form.data->uuid;
        in >> form.data->user;
        in >> form.data->secret;
        in >> form.data->name;
        in >> form.data->domain;
        in >> form.data->description;
        in >> form.data->created;
        in >> form.data->updated;
        in >> form.data->lockedAt;
        in >> form.data->lockedBy;
        in >> form.data->settings;
    }

    return in;
}

#include "moc_form.cpp"
