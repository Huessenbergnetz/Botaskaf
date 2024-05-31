/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_FORM_H
#define HBNBOTA_FORM_H

#include "objects/user.h"

#include <QDateTime>
#include <QObject>
#include <QSharedDataPointer>

class Error;

namespace Cutelyst {
class Context;
} // namespace Cutelyst

/*!
 * \brief Contains data about a contact form and provides form management methods.
 */
class Form
{
    Q_GADGET
    Q_PROPERTY(Form::dbid_t id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString domain READ domain CONSTANT)
    Q_PROPERTY(User owner READ owner CONSTANT)
    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(QString secret READ secret CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
    Q_PROPERTY(QDateTime lockedAt READ lockedAt CONSTANT)
    Q_PROPERTY(User lockedBy READ lockedBy CONSTANT)
    Q_PROPERTY(QVariantMap settings READ settings CONSTANT)
    Q_PROPERTY(QVariantMap urls READ urls CONSTANT)
    Q_PROPERTY(qint32 recipientCount READ recipientCount CONSTANT)
    Q_PROPERTY(bool hasRecipients READ hasRecipients CONSTANT)
public:
    using dbid_t = quint32;

    Form() noexcept = default;
    Form(dbid_t id,
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
         qint32 recipientCount);

    Form(const Form &other) noexcept            = default;
    Form(Form &&other) noexcept                 = default;
    Form &operator=(const Form &other) noexcept = default;
    Form &operator=(Form &&other) noexcept      = default;
    ~Form() noexcept                            = default;

    void swap(Form &other) noexcept { data.swap(other.data); }

    [[nodiscard]] dbid_t id() const noexcept;

    [[nodiscard]] QString uuid() const noexcept;

    [[nodiscard]] User owner() const noexcept;

    [[nodiscard]] QString secret() const noexcept;

    [[nodiscard]] QString name() const noexcept;

    [[nodiscard]] QString domain() const noexcept;

    [[nodiscard]] QString description() const noexcept;

    [[nodiscard]] QDateTime created() const noexcept;

    [[nodiscard]] QDateTime updated() const noexcept;

    [[nodiscard]] QDateTime lockedAt() const noexcept;

    [[nodiscard]] User lockedBy() const noexcept;

    [[nodiscard]] QVariantMap settings() const noexcept;

    [[nodiscard]] QVariantMap urls() const noexcept;

    [[nodiscard]] qint32 recipientCount() const noexcept;

    [[nodiscard]] bool hasRecipients() const noexcept;

    [[nodiscard]] bool isValid() const noexcept;

    [[nodiscard]] bool isNull() const noexcept { return !data; };

    [[nodiscard]] bool canEdit(const User &user) const;

    [[nodiscard]] bool canEdit(Cutelyst::Context *c) const;

    void clear()
    {
        if (!isNull())
            *this = Form();
    }

    /*!
     * \brief Returns \a id casted into dbid_t.
     *
     * Returns \c 0 if the value would overflow the dbid_t.
     *
     * If \a ok is not \c nullptr, failure is reported by setting \a *ok to \c false,
     * and success by setting \a *ok to \c true.
     */
    static dbid_t toDbId(qulonglong id, bool *ok = nullptr);

    /*!
     * \brief Returns \a var converted to dbid_t.
     *
     * Returns \c 0 if the conversion fails.
     *
     * If \a ok is not \c nullptr, failure is reported by setting \a *ok to \c false,
     * and success by setting \a *ok to \c true.
     */
    static dbid_t toDbId(const QVariant &var, bool *ok = nullptr);

    static QMap<QString, QString> labels(Cutelyst::Context *c);

    static Form fromStash(Cutelyst::Context *c);

    void toStash(Cutelyst::Context *c) const;

    static Form create(Cutelyst::Context *c, Error &e, const QVariantHash &values);

    static QList<Form> list(Cutelyst::Context *c, Error &e);

    static Form get(Cutelyst::Context *c, Error &e, Form::dbid_t id);

    static Form get(Cutelyst::Context *c, Error &e, const QString &uuid);

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;
        Data(Form::dbid_t _id,
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
             qint32 _recipientCount);

        Data(const Data &) noexcept   = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept              = default;

        void setUrls(Cutelyst::Context *c);

        User owner;
        User lockedBy;
        QString uuid;
        QString secret;
        QString name;
        QString domain;
        QString description;
        QDateTime created;
        QDateTime updated;
        QDateTime lockedAt;
        QVariantMap settings;
        QVariantMap urls;
        Form::dbid_t id{0};
        qint32 recipientCount{0};
    };

    QSharedDataPointer<Data> data;

    static Form fromCache(Form::dbid_t id);
    static Form fromCache(const QString &uuid);
    void toCache() const;

    friend QDataStream &operator<<(QDataStream &out, const Form &form);
    friend QDataStream &operator>>(QDataStream &in, Form &form);
};

Q_DECLARE_SHARED(Form) // NOLINT(modernize-type-traits)
Q_DECLARE_METATYPE(Form)

/*!
 * \related Form
 * \brief Writes the \a form to the debug stream \a dbg and returns a refererence to the stream.
 */
QDebug operator<<(QDebug dbg, const Form &form);

/*!
 * \related Form
 * \brief Writes \a form to stream \a out. Returns a reference to the stream.
 */
QDataStream &operator<<(QDataStream &out, const Form &form);

/*!
 * \related Form
 * \brief Reads a form from stream \a in into \a form. Returns a reference to the stream.
 */
QDataStream &operator>>(QDataStream &in, Form &form);

#endif // HBNBOTA_FORM_H
