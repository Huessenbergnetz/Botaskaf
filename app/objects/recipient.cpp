/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "recipient.h"

#include "logging.h"
#include "objects/error.h"
#include "settings.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Memcached/memcached.h>
#include <Cutelyst/Plugins/Utils/sql.h>

#include <QJsonDocument>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

using namespace Qt::Literals::StringLiterals;

#define HBNBOTA_RECIPIENT_STASH_KEY u"current_recipient"_s
#define HBNBOTA_RECIPIENT_MEMC_GROUP_KEY "recipients"_ba

Recipient::Data::Data(Recipient::dbid_t _id,
                      Form _form,
                      QString _fromName,
                      QString _fromEmail,
                      QString _toName,
                      QString _toEmail,
                      QString _subject,
                      QString _text,
                      QString _html,
                      QVariantMap _settings,
                      QDateTime _created,
                      QDateTime _updated,
                      QDateTime _lockedAt,
                      User _lockedBy)
    : QSharedData()
    , form{std::move(_form)}
    , lockedBy{std::move(_lockedBy)}
    , fromName{std::move(_fromName)}
    , fromEmail{std::move(_fromEmail)}
    , toName{std::move(_toName)}
    , toEmail{std::move(_toEmail)}
    , subject{std::move(_subject)}
    , text{std::move(_text)}
    , html{std::move(_html)}
    , settings{std::move(_settings)}
    , created{std::move(_created)}
    , updated{std::move(_updated)}
    , lockedAt{std::move(_lockedAt)}
    , id{_id}
{
    created.setTimeSpec(Qt::UTC);
    if (updated.isValid()) {
        updated.setTimeSpec(Qt::UTC);
    }
    if (lockedAt.isValid()) {
        lockedAt.setTimeSpec(Qt::UTC);
    }
}

void Recipient::Data::setUrls(Cutelyst::Context *c)
{
    Q_UNUSED(c)
}

Recipient::Recipient(Recipient::dbid_t id,
                     const Form &form,
                     const QString &fromName,
                     const QString &fromEmail,
                     const QString &toName,
                     const QString &toEmail,
                     const QString &subject,
                     const QString &text,
                     const QString &html,
                     const QVariantMap &settings,
                     const QDateTime &created,
                     const QDateTime &updated,
                     const QDateTime &lockedAt,
                     const User &lockedBy)
    : data{new Recipient::Data{id,
                               form,
                               fromName,
                               fromEmail,
                               toName,
                               toEmail,
                               subject,
                               text,
                               html,
                               settings,
                               created,
                               updated,
                               lockedAt,
                               lockedBy}}
{
}

Recipient::dbid_t Recipient::id() const noexcept
{
    return data ? data->id : 0;
}

Form Recipient::form() const noexcept
{
    return data ? data->form : Form();
}

QString Recipient::fromName() const noexcept
{
    return data ? data->fromName : QString();
}

QString Recipient::fromEmail() const noexcept
{
    return data ? data->fromEmail : QString();
}

QString Recipient::toName() const noexcept
{
    return data ? data->toName : QString();
}

QString Recipient::toEmail() const noexcept
{
    return data ? data->toEmail : QString();
}

QString Recipient::subject() const noexcept
{
    return data ? data->subject : QString();
}

QString Recipient::text() const noexcept
{
    return data ? data->text : QString();
}

QString Recipient::html() const noexcept
{
    return data ? data->html : QString();
}

QVariantMap Recipient::settings() const noexcept
{
    return data ? data->settings : QVariantMap();
}

QDateTime Recipient::created() const noexcept
{
    return data ? data->created : QDateTime();
}

QDateTime Recipient::updated() const noexcept
{
    return data ? data->updated : QDateTime();
}

QDateTime Recipient::lockedAt() const noexcept
{
    return data ? data->lockedAt : QDateTime();
}

User Recipient::lockedBy() const noexcept
{
    return data ? data->lockedBy : User();
}

bool Recipient::isValid() const noexcept
{
    return data && data->id > 0;
}

Recipient::dbid_t Recipient::toDbId(qulonglong id, bool *ok)
{
    if (id > static_cast<qulonglong>(std::numeric_limits<Recipient::dbid_t>::max())) {
        if (ok) {
            *ok = false;
        }
        return 0;
    }

    if (ok) {
        *ok = true;
    }

    return static_cast<Recipient::dbid_t>(id);
}

Recipient::dbid_t Recipient::toDbId(const QVariant &var, bool *ok)
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

QMap<QString, QString> Recipient::labels(Cutelyst::Context *c)
{
    return {//: Recipient data label, used eg. in table headers
            //% "id"
            {u"id"_s, c->qtTrId("hbnbota_recipient_label_id")},
            //: Recipient data label, used eg. in table headers
            //% "form"
            {u"form"_s, c->qtTrId("hbnbota_recipient_label_form")},
            //: Recipient data label, used eg. in table headers
            //% "from"
            {u"from"_s, c->qtTrId("hbnbota_recipient_label_from")},
            //: Recipient data label, used eg. in table headers
            //% "to"
            {u"to"_s, c->qtTrId("hbnbota_recipient_label_to")},
            //: General data label, used eg. in table headers, means the creation date and time
            //% "created"
            {u"created"_s, c->qtTrId("hbnbota_general_label_created")},
            //: General data label, used eg. in table headers,
            //: means the date and time an object has been updated the last time
            //% "updated"
            {u"updated"_s, c->qtTrId("hbnbota_general_label_updated")}};
}

Recipient Recipient::create(Cutelyst::Context *c, const Form &form, Error &e, const QVariantHash &values)
{
    const auto fromName  = values.value(u"fromName"_s).toString();
    const auto fromEmail = values.value(u"fromEmail"_s).toString();
    const auto toName    = values.value(u"toName"_s).toString();
    const auto toEmail   = values.value(u"toEmail"_s).toString();
    const auto subject   = values.value(u"fromName"_s).toString();
    const auto text      = values.value(u"text"_s).toString();
    const auto html      = values.value(u"html"_s).toString();
    const auto now       = QDateTime::currentDateTimeUtc();
    QVariantMap settings;
    QVariantMap replyTo;
    replyTo.insert(u"name"_s, values.value(u"replyToName"_s).toString());
    replyTo.insert(u"email"_s, values.value(u"replyToEmail"_s).toString());
    settings.insert(u"replyTo"_s, replyTo);
    const QByteArray jsonSettings = QJsonDocument(QJsonObject::fromVariantMap(settings)).toJson(QJsonDocument::Compact);

    QSqlQuery q = CPreparedSqlQueryThread(
        u"INSERT INTO recipients (formId, fromName, fromEmail, toName, toEmail, subject, text, html, settings, created) "
        "VALUES (:formId, :fromName, :fromEmail, :toName, :toEmail, :subject, :text, :html, :settings, :created)"_s);
    if (Q_UNLIKELY(q.lastError().isValid())) {
        //: Error message
        //% "Failed to insert new recipient into database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_recipient_failed_create_db"));
        qCCritical(HBNBOTA_CORE) << "Failed to insert new recipient into database:" << q.lastError().text();
        return {};
    }

    q.bindValue(u":formId"_s, form.id());
    q.bindValue(u":fromName"_s, fromName);
    q.bindValue(u":fromEmail"_s, fromEmail);
    q.bindValue(u":toName"_s, toName);
    q.bindValue(u":toEmail"_s, toEmail);
    q.bindValue(u":subject"_s, subject);
    q.bindValue(u":text"_s, text);
    q.bindValue(u":html"_s, html);
    q.bindValue(u":settings"_s, jsonSettings);
    q.bindValue(u":created"_s, now);

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(c, q, c->qtTrId("hbnbota_error_recipient_failed_create_db"));
        qCCritical(HBNBOTA_CORE) << "Failed to insert new recipient into database:" << q.lastError().text();
        return {};
    }

    Recipient::dbid_t id = 0;
    if (q.driver()->hasFeature(QSqlDriver::LastInsertId)) {
        id = Recipient::toDbId(q.lastInsertId());
    } else {
        q = CPreparedSqlQueryThreadFO(u"SELECT id FROM recipients WHERE formId = :formId AND toEmail = :toEmail"_s);
        q.bindValue(u":formId"_s, form.id());
        q.bindValue(u":toEmail"_s, toEmail);
        q.exec();
        q.next();
        id = Recipient::toDbId(q.value(0));
    }

    Recipient r{id, form, fromName, fromEmail, toName, toEmail, subject, text, html, settings, now, {}, {}, {}};
    r.data->setUrls(c);
    r.toCache();

    qCInfo(HBNBOTA_CORE) << User::fromStash(c) << "created new" << r;

    return r;
}

QList<Recipient> Recipient::list(Cutelyst::Context *c, const Form &form, Error &e)
{
    QSqlQuery q = CPreparedSqlQueryThreadFO(
        u"SELECT id, fromName, fromEmail, toName, toEmail, subject, text, html, settings, created, updated, lockedAt, lockedBy FROM recipients WHERE formId = :formId"_s);
    if (Q_UNLIKELY(q.lastError().isValid())) {
        //: Error message, %1 will be replaced by the form name, %2 by the form db id
        //% "Failed to query recipients for form “%1” (ID: %2) from the database."
        e = Error::create(
            c, q, c->qtTrId("hbnbota_error_recipient_failed_list_db").arg(form.name(), QString::number(form.id())));
        qCCritical(HBNBOTA_CORE) << "Failed to query recipients for" << form << "from database:" << q.lastError().text();
        return {};
    }

    q.bindValue(u":formId"_s, form.id());

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(
            c, q, c->qtTrId("hbnbota_error_recipient_failed_list_db").arg(form.name(), QString::number(form.id())));
        qCCritical(HBNBOTA_CORE) << "Failed to query recipients for" << form << "from database:" << q.lastError().text();
        return {};
    }

    QList<Recipient> lst;
    if (q.size() > 0) {
        lst.reserve(q.size());
    }

    while (q.next()) {
        User lockedBy;
        if (const auto lockedById = User::toDbId(q.value(12)); lockedById > 0) {
            Error _e;
            lockedBy = User::get(c, _e, lockedById);
        }

        auto &r = lst.emplace_back(Recipient::toDbId(q.value(0)),
                                   form,
                                   q.value(1).toString(),
                                   q.value(2).toString(),
                                   q.value(3).toString(),
                                   q.value(4).toString(),
                                   q.value(5).toString(),
                                   q.value(6).toString(),
                                   q.value(7).toString(),
                                   QJsonDocument::fromJson(q.value(8).toByteArray()).object().toVariantMap(),
                                   q.value(9).toDateTime(),
                                   q.value(10).toDateTime(),
                                   q.value(11).toDateTime(),
                                   lockedBy);
        r.data->setUrls(c);
    }

    return lst;
}

Recipient Recipient::fromCache(Recipient::dbid_t id)
{
    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::ReturnType rt{Cutelyst::Memcached::ReturnType::Failure};
        auto r =
            Cutelyst::Memcached::getByKey<Recipient>(HBNBOTA_RECIPIENT_MEMC_GROUP_KEY, QByteArray::number(id), nullptr, &rt);
        if (rt == Cutelyst::Memcached::ReturnType::Success) {
            qCDebug(HBNBOTA_CORE) << "Found recipient with ID" << id << "in memcached";
            return r;
        }
    }

    return {};
}

void Recipient::toCache() const
{
    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::setByKey<Recipient>(
            HBNBOTA_RECIPIENT_MEMC_GROUP_KEY, QByteArray::number(id()), *this, std::chrono::days{7});
    }
}

QDebug operator<<(QDebug dbg, const Recipient &recipient)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Recipient(";
    if (!recipient.isNull()) {
        if (recipient.isValid()) {
            dbg << "ID: " << recipient.id();
            dbg << ", Form ID: " << recipient.form().id();
            dbg << ", From: ";
            if (recipient.fromName().isEmpty()) {
                dbg << recipient.fromEmail();
            } else {
                dbg << recipient.fromName() << " <" << recipient.fromEmail() << '>';
            }
            dbg << ", To: ";
            if (recipient.toName().isEmpty()) {
                dbg << recipient.toEmail();
            } else {
                dbg << recipient.toName() << " <" << recipient.toEmail() << '>';
            }
            dbg << ", Subject: " << recipient.subject();
        } else {
            dbg << "INVALID";
        }
    }
    dbg << ')';
    return dbg;
}

QDataStream &operator<<(QDataStream &out, const Recipient &recipient)
{
    if (!recipient.isNull()) {
        out << recipient.data->id << recipient.data->form << recipient.data->fromName << recipient.data->fromEmail
            << recipient.data->toName << recipient.data->toEmail << recipient.data->subject << recipient.data->text
            << recipient.data->html << recipient.data->settings << recipient.data->created << recipient.data->updated
            << recipient.data->lockedAt << recipient.data->lockedBy;
    } else {
        out << static_cast<Recipient::dbid_t>(0);
    }

    return out;
}

QDataStream &operator>>(QDataStream &in, Recipient &recipient)
{
    Recipient::dbid_t id = 0;
    in >> id;

    if (id == 0) {
        recipient.clear();
    } else {
        if (recipient.data == nullptr) {
            recipient.data = new Recipient::Data;
        }

        recipient.data->id = id;
        in >> recipient.data->form;
        in >> recipient.data->fromName;
        in >> recipient.data->fromEmail;
        in >> recipient.data->toName;
        in >> recipient.data->toEmail;
        in >> recipient.data->subject;
        in >> recipient.data->text;
        in >> recipient.data->html;
        in >> recipient.data->settings;
        in >> recipient.data->created;
        in >> recipient.data->updated;
        in >> recipient.data->lockedAt;
        in >> recipient.data->lockedBy;
    }

    return in;
}

#include "moc_recipient.cpp"
