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
    Q_PROPERTY(User user READ user CONSTANT)
    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(QString secret READ secret CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
    Q_PROPERTY(QDateTime lockedAt READ lockedAt CONSTANT)
    Q_PROPERTY(User lockedBy READ lockedBy CONSTANT)
    Q_PROPERTY(QVariantMap settings READ settings CONSTANT)
    Q_PROPERTY(QUrl editUrl READ editUrl CONSTANT)
    Q_PROPERTY(QUrl removeUrl READ removeUrl CONSTANT)
public:
    using dbid_t = quint32;

    Form() noexcept = default;
    Form(dbid_t id,
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
         const QVariantMap &settings);

    Form(const Form &other) noexcept            = default;
    Form(Form &&other) noexcept                 = default;
    Form &operator=(const Form &other) noexcept = default;
    Form &operator=(Form &&other) noexcept      = default;
    ~Form() noexcept                            = default;

    void swap(Form &other) noexcept { data.swap(other.data); }

    [[nodiscard]] dbid_t id() const noexcept;

    [[nodiscard]] QString uuid() const noexcept;

    [[nodiscard]] User user() const noexcept;

    [[nodiscard]] QString secret() const noexcept;

    [[nodiscard]] QString name() const noexcept;

    [[nodiscard]] QString domain() const noexcept;

    [[nodiscard]] QString description() const noexcept;

    [[nodiscard]] QDateTime created() const noexcept;

    [[nodiscard]] QDateTime updated() const noexcept;

    [[nodiscard]] QDateTime lockedAt() const noexcept;

    [[nodiscard]] User lockedBy() const noexcept;

    [[nodiscard]] QVariantMap settings() const noexcept;

    [[nodiscard]] QUrl editUrl() const noexcept;

    [[nodiscard]] QUrl removeUrl() const noexcept;

    [[nodiscard]] bool isValid() const noexcept;

    [[nodiscard]] bool isNull() const noexcept { return !data; };

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

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;
        Data(Form::dbid_t _id,
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
             const QVariantMap &_settings);

        Data(const Data &) noexcept   = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept              = default;

        void setUrls(Cutelyst::Context *c);

        User user;
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
        QUrl editUrl;
        QUrl removeUrl;
        Form::dbid_t id{0};
    };

    QSharedDataPointer<Data> data;

    static Form fromCache(Form::dbid_t id);
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
