/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "form.h"

#include "logging.h"
#include "objects/error.h"
#include "settings.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Memcached/memcached.h>
#include <Cutelyst/Plugins/Utils/sql.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;

#define HBNBOTA_FORM_STASH_KEY u"contactform"_s
#define HBNBOTA_FORM_MEMC_GROUP_KEY "forms"_ba

Form::Data::Data(Form::dbid_t _id,
                 const QString &_name,
                 const QString &_domain,
                 const User &_user,
                 const QString &_uuid,
                 const QString &_secret,
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
           const QString &name,
           const QString &domain,
           const User &user,
           const QString &uuid,
           const QString &secret,
           const QString &description,
           const QDateTime &created,
           const QDateTime &updated,
           const QDateTime &lockedAt,
           const User &lockedBy,
           const QVariantMap &settings)
    : data{new Form::Data{id, name, domain, user, uuid, secret, description, created, updated, lockedAt, lockedBy, settings}}
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
    const User user = User::fromStash(c);
    if (!user.isValid()) {
        return {};
    }

    const QString uuid        = QUuid::createUuid().toString(QUuid::Id128).toLower();
    const QString secret      = QUuid::createUuid().toString(QUuid::Id128).toUpper();
    const QString name        = values.value(u"name"_s).toString();
    const QString domain      = values.value(u"domain"_s).toString();
    const QString description = values.value(u"description"_s).toString();
    const QDateTime now       = QDateTime::currentDateTimeUtc();
    QVariantMap settings;
    settings.insert(u"honeypots"_s, values.value(u"honeypots"_s).toString().split(','_L1, Qt::SkipEmptyParts));
    QVariantMap fields;
    fields.insert(u"name"_s,
                  QVariantMap({{u"name"_s, values.value(u"formFieldSenderName"_s)},
                               {u"required"_s, values.value(u"formFieldSenderNameRequired"_s, false)}}));
    fields.insert(u"email"_s,
                  QVariantMap({{u"name"_s, values.value(u"formFieldSenderEmail"_s)},
                               {u"required"_s, values.value(u"formFieldSenderEmailRequired"_s, false)}}));
    fields.insert(u"phone"_s,
                  QVariantMap({{u"name"_s, values.value(u"formFieldSenderPhone"_s)},
                               {u"required"_s, values.value(u"formFieldSenderPhoneRequired"_s, false)}}));
    fields.insert(u"url"_s,
                  QVariantMap({{u"name"_s, values.value(u"formFieldSenderUrl"_s)},
                               {u"required"_s, values.value(u"formFieldSenderUrlRequired"_s, false)}}));
    fields.insert(u"subject"_s,
                  QVariantMap({{u"name"_s, values.value(u"formFieldSubject"_s)},
                               {u"required"_s, values.value(u"formFieldSubjectRequired"_s, false)}}));
    fields.insert(u"content"_s,
                  QVariantMap({{u"name"_s, values.value(u"formFieldContent"_s)},
                               {u"required"_s, values.value(u"formFieldContentRequired"_s, false)}}));
    settings.insert(u"fields"_s, fields);
    QVariantMap mailer;
    mailer.insert(u"type"_s, values.value(u"senderType"_s));
    QVariantMap smtp;
    smtp.insert(u"host"_s, values.value(u"smtpHost"_s));
    smtp.insert(u"port"_s, values.value(u"smtpPort"_s));
    smtp.insert(u"user"_s, values.value(u"smtpUser"_s));
    smtp.insert(u"password"_s, values.value(u"smtpPassword"_s));
    smtp.insert(u"encryption"_s, values.value(u"smtpEncryption"_s));
    smtp.insert(u"authentication"_s, values.value(u"smtpAuthentication"_s));
    mailer.insert(u"smtp"_s, smtp);
    settings.insert(u"mailer"_s, mailer);
    const QByteArray jsonSettings = QJsonDocument(QJsonObject::fromVariantMap(settings)).toJson(QJsonDocument::Compact);

    QSqlQuery q =
        CPreparedSqlQueryThread(u"INSERT INTO forms (name, domain, userId, uuid, secret, description, created, settings) "
                                "VALUES (:name, :domain, :userId, :uuid, :secret, :description, :created, :settings)"_s);
    if (Q_UNLIKELY(q.lastError().isValid())) {
        //: Error message, %1 will be replaced by the form name
        //% "Failed to insert new form “%1” into database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_failed_create_db").arg(name));
        qCCritical(HBNBOTA_CORE) << "Failed to insert new form" << name << "into database:" << q.lastError().text();
        return {};
    }

    q.bindValue(u":name"_s, name);
    q.bindValue(u":domain"_s, domain);
    q.bindValue(u":userId"_s, user.id());
    q.bindValue(u":uuid"_s, uuid);
    q.bindValue(u":secret"_s, secret);
    q.bindValue(u":description"_s, description);
    q.bindValue(u":created"_s, now);
    q.bindValue(u":settings"_s, jsonSettings);

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_failed_create_db").arg(name));
        qCCritical(HBNBOTA_CORE) << "Failed to insert new form" << name << "into database:" << q.lastError().text();
        return {};
    }

    Form::dbid_t id = 0;

    if (q.driver()->hasFeature(QSqlDriver::LastInsertId)) {
        id = Form::toDbId(q.lastInsertId());
    } else {
        q = CPreparedSqlQueryThreadFO(u"SELECT id FROM forms WHERE uuid = :uuid"_s);
        q.bindValue(u":uuid"_s, uuid);
        q.exec();
        q.next();
        id = Form::toDbId(q.value(0));
    }

    Form f{id, name, domain, user, uuid, secret, description, now, {}, {}, {}, settings};
    f.toCache();

    qCInfo(HBNBOTA_CORE) << user << "created new" << f;

    return f;
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
