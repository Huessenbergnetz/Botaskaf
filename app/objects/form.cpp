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
#include <botan/auto_rng.h>
#include <botan/cipher_mode.h>
#include <botan/hex.h>
#include <botan/rng.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;

#define HBNBOTA_FORM_STASH_KEY u"current_form"_s
#define HBNBOTA_FORMBYID_MEMC_GROUP_KEY "formsbyid"_ba
#define HBNBOTA_FORMBYUUID_MEMC_GROUP_KEY "formsbyuuid"_ba

Form::Data::Data(Form::dbid_t _id,
                 const QString &_name,
                 const QString &_domain,
                 const User &_owner,
                 const QString &_uuid,
                 const QString &_secret,
                 const QString &_description,
                 const QDateTime &_created,
                 const QDateTime &_updated,
                 const QDateTime &_lockedAt,
                 const User &_lockedBy,
                 const QVariantMap &_settings,
                 qint32 _recipientCount)
    : QSharedData()
    , owner{_owner}
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
    , recipientCount{_recipientCount}
{
    created.setTimeSpec(Qt::UTC);
    updated.setTimeSpec(Qt::UTC);
    if (lockedAt.isValid()) {
        lockedAt.setTimeSpec(Qt::UTC);
    }
}

void Form::Data::setUrls(Cutelyst::Context *c)
{
    const auto currentUser = User::fromStash(c);
    if (currentUser.isAdmin() || currentUser == owner) {
        const QStringList _id = {QString::number(id)};
        urls.insert(u"edit"_s, c->uriForAction(u"/forms/editForm", _id));
        urls.insert(u"remoive"_s, c->uriForAction(u"/forms/removeForm", _id));
        urls.insert(u"recipients"_s, c->uriForAction(u"/forms/recipients", _id));
        urls.insert(u"addRecipient"_s, c->uriForAction(u"/forms/addRecipient", _id));
    }
}

Form::Form(dbid_t id,
           const QString &name,
           const QString &domain,
           const User &owner,
           const QString &uuid,
           const QString &secret,
           const QString &description,
           const QDateTime &created,
           const QDateTime &updated,
           const QDateTime &lockedAt,
           const User &lockedBy,
           const QVariantMap &settings,
           qint32 recipientCount)
    : data{new Form::Data{id,
                          name,
                          domain,
                          owner,
                          uuid,
                          secret,
                          description,
                          created,
                          updated,
                          lockedAt,
                          lockedBy,
                          settings,
                          recipientCount}}
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

User Form::owner() const noexcept
{
    return data ? data->owner : User();
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

QVariantMap Form::urls() const noexcept
{
    return data ? data->urls : QVariantMap();
}

qint32 Form::recipientCount() const noexcept
{
    return data ? data->recipientCount : 0;
}

bool Form::hasRecipients() const noexcept
{
    return data ? data->recipientCount > 0 : false;
}

bool Form::isValid() const noexcept
{
    return data && data->id > 0;
}

bool Form::canEdit(const User &user) const
{
    if (user.isNull()) {
        return false;
    }

    if (user.isAdmin()) {
        return true;
    }

    return data && data->owner.id() == user.id();
}

bool Form::canEdit(Cutelyst::Context *c) const
{
    return canEdit(User::fromStash(c));
}

QByteArray Form::encrypt(const QDateTime &dt) const
{
    if (!data) {
        qCCritical(HBNBOTA_CORE) << "Failed to encrypt token, invalid Form object";
        return {};
    }

    if (!dt.isValid()) {
        qCCritical(HBNBOTA_CORE) << "Failed to encrypt token, invalid date and time";
        return {};
    }

    const auto ba = QByteArray::number(dt.toMSecsSinceEpoch());

    Botan::AutoSeeded_RNG rng;

    const QByteArray sec           = secret().toUtf8();
    const std::vector<uint8_t> key = Botan::hex_decode(sec.constData());

    const auto enc = Botan::Cipher_Mode::create("AES-128/GCM", Botan::Cipher_Dir::ENCRYPTION);
    if (!enc) {
        qCCritical(HBNBOTA_CORE) << "Failed to encrypt token, can not create Botan::Cipher_Mode object";
        return {};
    }

    enc->set_key(key);

    Botan::secure_vector<uint8_t> iv = rng.random_vec(enc->default_nonce_length());
    Botan::secure_vector<uint8_t> t{ba.constData(), ba.constData() + ba.length()};

    enc->start(iv);
    enc->finish(t);

    return QByteArray::fromStdString(Botan::hex_encode(iv)) + ":"_ba + QByteArray::fromStdString(Botan::hex_encode(t));
}

QDateTime Form::decrypt(const QByteArray &ba) const
{
    if (!data) {
        qCCritical(HBNBOTA_CORE) << "Failed to decrypt token, invalid Form object";
        return {};
    }

    const int colonPos = ba.indexOf(':');
    if (colonPos < 1) {
        qCCritical(HBNBOTA_CORE) << "Failed to decrypt token, invalid input data";
        return {};
    }

    const QByteArray ivBa = ba.left(colonPos);
    qCDebug(HBNBOTA_CORE) << "IV:" << ivBa;
    const QByteArray dataBa = ba.mid(colonPos + 1);
    qCDebug(HBNBOTA_CORE) << "DATA:" << dataBa;

    Botan::AutoSeeded_RNG rng;

    const QByteArray sec           = secret().toUtf8();
    const std::vector<uint8_t> key = Botan::hex_decode(sec.constData());

    const auto dec = Botan::Cipher_Mode::create("AES-128/GCM", Botan::Cipher_Dir::DECRYPTION);
    if (!dec) {
        qCCritical(HBNBOTA_CORE) << "Failed to decrypt token, can not create Botan::Cipher_Mode object";
        return {};
    }

    dec->set_key(key);

    Botan::secure_vector<uint8_t> iv = Botan::hex_decode_locked(ivBa.toStdString());
    Botan::secure_vector<uint8_t> t  = Botan::hex_decode_locked(dataBa.toStdString());

    dec->start(iv);
    dec->finish(t);

    QByteArray outBa;
    for (const auto c : t) {
        outBa.append(static_cast<char>(c));
    }

    return QDateTime::fromMSecsSinceEpoch(outBa.toLongLong(), QTimeZone::utc());
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

QMap<QString, QString> Form::labels(Cutelyst::Context *c)
{
    return {//: Form data label, used eg. in table headers
            //% "id"
            {u"id"_s, c->qtTrId("hbnbota_form_label_id")},
            //: Form data label, used eg. in table headers
            //% "name"
            {u"name"_s, c->qtTrId("hbnbota_form_label_name")},
            //: Form data label, used eg. in table headers
            //% "domain"
            {u"domain"_s, c->qtTrId("hbnbota_form_label_domain")},
            //: Form data label, used eg. in table headers
            //% "owner"
            {u"owner"_s, c->qtTrId("hbnbota_form_label_owner")},
            //: Form data label, used eg. in table headers
            //% "created"
            {u"created"_s, c->qtTrId("hbnbota_form_label_created")},
            //: Form data label, used eg. in table headers
            //% "updated"
            {u"updated"_s, c->qtTrId("hbnbota_form_label_updated")},
            //: Form data label, used eg. in table headers
            //% "recipients"
            {u"recipientCount"_s, c->qtTrId("hbnbota_form_label_recipients")}};
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

    Form f{id, name, domain, user, uuid, secret, description, now, {}, {}, {}, settings, 0};
    f.data->setUrls(c);
    f.toCache();

    qCInfo(HBNBOTA_CORE) << user << "created new" << f;

    return f;
}

QList<Form> Form::list(Cutelyst::Context *c, Error &e)
{
    auto user = User::fromStash(c);
    QSqlQuery q;
    if (user.isAdmin()) {
        q = CPreparedSqlQueryThreadFO(
            u"SELECT f.id, f.name, f.domain, f.userId, f.uuid, f.secret, f.description, f.created, f.updated, f.lockedAt, "
            "f.lockedBy, f.settings, (SELECT COUNT(*) FROM recipients r WHERE r.formId = f.id) AS recipientCount "
            "FROM forms f"_s);
    } else {
        q = CPreparedSqlQueryThreadFO(
            u"SELECT f.id, f.name, f.domain, f.userId, f.uuid, f.secret, f.description, f.created, f.updated, f.lockedAt, "
            "f.lockedBy, f.settings, (SELECT COUNT(*) FROM recipients r WHERE r.formId = f.id) AS recipientCount "
            "FROM forms WHERE userId = :userId"_s);
    }

    if (Q_UNLIKELY(q.lastError().isValid())) {
        //: Error message
        //% "Failed to query forms from database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_list_query_failed"));
        qCCritical(HBNBOTA_CORE) << "Failed to query forms from database:" << q.lastError().text();
        return {};
    }

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_list_query_failed"));
        qCCritical(HBNBOTA_CORE) << "Failed to query forms from database:" << q.lastError().text();
        return {};
    }

    QList<Form> lst;
    if (q.size() > 0) {
        lst.reserve(q.size());
    }

    if (user.isAdmin()) {
        while (q.next()) {
            User owner;
            if (const auto ownerId = User::toDbId(q.value(3)); ownerId > 0) {
                if (ownerId == user.id()) {
                    owner = user;
                } else {
                    Error _e;
                    owner = User::get(c, _e, ownerId);
                }
            }

            User lockedBy;
            if (const auto lockedById = User::toDbId(q.value(10)); lockedById > 0) {
                Error _e;
                lockedBy = User::get(c, _e, lockedById);
            }

            auto &f = lst.emplace_back(Form::toDbId(q.value(0)),
                                       q.value(1).toString(),
                                       q.value(2).toString(),
                                       owner,
                                       q.value(4).toString(),
                                       q.value(5).toString(),
                                       q.value(6).toString(),
                                       q.value(7).toDateTime(),
                                       q.value(8).toDateTime(),
                                       q.value(9).toDateTime(),
                                       lockedBy,
                                       QJsonDocument::fromJson(q.value(11).toByteArray()).object().toVariantMap(),
                                       q.value(12).toInt());
            f.data->setUrls(c);
        }
    } else {
        while (q.next()) {
            User lockedBy;
            if (const auto lockedById = User::toDbId(q.value(10)); lockedById > 0) {
                Error _e;
                lockedBy = User::get(c, _e, lockedById);
            }

            auto &f = lst.emplace_back(Form::toDbId(q.value(0)),
                                       q.value(1).toString(),
                                       q.value(2).toString(),
                                       user,
                                       q.value(4).toString(),
                                       q.value(5).toString(),
                                       q.value(6).toString(),
                                       q.value(7).toDateTime(),
                                       q.value(8).toDateTime(),
                                       q.value(9).toDateTime(),
                                       lockedBy,
                                       QJsonDocument::fromJson(q.value(11).toByteArray()).object().toVariantMap(),
                                       q.value(12).toInt());
            f.data->setUrls(c);
        }
    }

    return lst;
}

Form getForm(Cutelyst::Context *c, QSqlQuery &q)
{
    const auto user = User::fromStash(c);
    User owner;
    if (const auto ownerId = User::toDbId(q.value(3)); ownerId > 0) {
        if (ownerId == user.id()) {
            owner = user;
        } else {
            Error _e;
            owner = User::get(c, _e, ownerId);
        }
    }
    User lockedBy;
    if (const auto lockedById = User::toDbId(q.value(10)); lockedById > 0) {
        if (lockedById == user.id()) {
            lockedBy = user;
        } else {
            Error _e;
            lockedBy = User::get(c, _e, lockedById);
        }
    }

    Form f{Form::toDbId(q.value(0)),
           q.value(1).toString(),
           q.value(2).toString(),
           owner,
           q.value(4).toString(),
           q.value(5).toString(),
           q.value(6).toString(),
           q.value(7).toDateTime(),
           q.value(8).toDateTime(),
           q.value(9).toDateTime(),
           lockedBy,
           QJsonDocument::fromJson(q.value(11).toByteArray()).object().toVariantMap(),
           q.value(12).toInt()};

    return f;
}

Form Form::get(Cutelyst::Context *c, Error &e, Form::dbid_t id)
{
    Form f = Form::fromCache(id);
    if (!f.isNull()) {
        return f;
    }

    qCDebug(HBNBOTA_CORE) << "Query form with ID" << id << "from the database";

    QSqlQuery q = CPreparedSqlQueryThreadFO(
        u"SELECT f.id, f.name, f.domain, f.userId, f.uuid, f.secret, f.description, f.created, f.updated, f.lockedAt, "
        "f.lockedBy, f.settings, (SELECT COUNT(*) FROM recipients r WHERE r.formId = f.id) AS recipientCount "
        "FROM forms f WHERE f.id = :id"_s);

    if (Q_UNLIKELY(q.lastError().isValid())) {
        //: Error message
        //% "Failed to get contact form with ID %1 from database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_getbyid_query_failed").arg(id));
        qCCritical(HBNBOTA_CORE) << "Failed to get form with ID" << id << "from database:" << q.lastError().text();
        return f;
    }

    q.bindValue(u":id"_s, id);

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_getbyid_query_failed").arg(id));
        qCCritical(HBNBOTA_CORE) << "Failed to get form with ID" << id << "from database:" << q.lastError().text();
        return f;
    }

    if (Q_UNLIKELY(!q.next())) {
        //: Error message
        //% "Can not find contact form with ID %1 in the database."
        e = Error::create(c, Cutelyst::Response::NotFound, c->qtTrId("hbnbota_error_form_getbyid_not_found").arg(id));
        qCCritical(HBNBOTA_CORE) << "Can not find contact form ID" << id << "in the databse";
        return f;
    }

    f = getForm(c, q);
    f.data->setUrls(c);
    f.toCache();
    return f;
}

Form Form::get(Cutelyst::Context *c, Error &e, const QString &uuid)
{
    Form f = Form::fromCache(uuid);
    if (!f.isNull()) {
        return f;
    }

    qCDebug(HBNBOTA_CORE) << "Query form with UUID" << uuid << "from the database";

    QSqlQuery q = CPreparedSqlQueryThreadFO(
        u"SELECT f.id, f.name, f.domain, f.userId, f.uuid, f.secret, f.description, f.created, f.updated, f.lockedAt, "
        "f.lockedBy, f.settings, (SELECT COUNT(*) FROM recipients r WHERE r.formId = f.id) AS recipientCount "
        "FROM forms f WHERE f.uuid = :uuid"_s);

    if (Q_UNLIKELY(q.lastError().isValid())) {
        //: Error message
        //% "Failed to get contact form with UUID “%1” from database."
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_getbyuuid_query_failed").arg(uuid));
        qCCritical(HBNBOTA_CORE) << "Failed to get form with UUID" << uuid << "from database:" << q.lastError().text();
        return f;
    }

    q.bindValue(u":uuid"_s, uuid);

    if (Q_UNLIKELY(!q.exec())) {
        e = Error::create(c, q, c->qtTrId("hbnbota_error_form_getbyuuid_query_failed").arg(uuid));
        qCCritical(HBNBOTA_CORE) << "Failed to get form with UUID" << uuid << "from database:" << q.lastError().text();
        return f;
    }

    if (Q_UNLIKELY(!q.next())) {
        //: Error message
        //% "Can not find contact form with UUID “%1” in the database."
        e = Error::create(c, Cutelyst::Response::NotFound, c->qtTrId("hbnbota_error_form_getbyuuid_not_found").arg(uuid));
        qCCritical(HBNBOTA_CORE) << "Can not find contact form UUID" << uuid << "in the databse";
        return f;
    }

    f = getForm(c, q);
    f.data->setUrls(c);
    f.toCache();
    return f;
}

Form Form::fromCache(Form::dbid_t id)
{
    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::ReturnType rt{Cutelyst::Memcached::ReturnType::Failure};
        const Form f =
            Cutelyst::Memcached::getByKey<Form>(HBNBOTA_FORMBYID_MEMC_GROUP_KEY, QByteArray::number(id), nullptr, &rt);
        if (rt == Cutelyst::Memcached::ReturnType::Success) {
            qCDebug(HBNBOTA_CORE) << "Found contact form with ID" << id << "in memcached";
            return f;
        }
    }

    return {};
}

Form Form::fromCache(const QString &uuid)
{
    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::ReturnType rt{Cutelyst::Memcached::ReturnType::Failure};
        const Form f = Cutelyst::Memcached::getByKey<Form>(HBNBOTA_FORMBYID_MEMC_GROUP_KEY, uuid.toUtf8(), nullptr, &rt);
        if (rt == Cutelyst::Memcached::ReturnType::Success) {
            qCDebug(HBNBOTA_CORE) << "Found contact form with UUID" << uuid << "in memcached";
            return f;
        }
    }

    return {};
}

void Form::toCache() const
{
    if (Settings::cache() == Settings::Cache::Memcached) {
        Cutelyst::Memcached::setByKey<Form>(
            HBNBOTA_FORMBYID_MEMC_GROUP_KEY, QByteArray::number(id()), *this, std::chrono::days{7});
        Cutelyst::Memcached::setByKey<Form>(HBNBOTA_FORMBYUUID_MEMC_GROUP_KEY, uuid().toUtf8(), *this, std::chrono::days{7});
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
        out << form.data->id << form.data->uuid << form.data->owner << form.data->secret << form.data->name
            << form.data->domain << form.data->description << form.data->created << form.data->updated << form.data->lockedAt
            << form.data->lockedBy << form.data->settings << form.data->urls << form.data->recipientCount;
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
        in >> form.data->owner;
        in >> form.data->secret;
        in >> form.data->name;
        in >> form.data->domain;
        in >> form.data->description;
        in >> form.data->created;
        in >> form.data->updated;
        in >> form.data->lockedAt;
        in >> form.data->lockedBy;
        in >> form.data->settings;
        in >> form.data->urls;
        in >> form.data->recipientCount;
    }

    return in;
}

#include "moc_form.cpp"
