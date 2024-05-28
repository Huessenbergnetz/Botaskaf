/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_RECIPIENT_H
#define HBNBOTA_RECIPIENT_H

#include "form.h"
#include "user.h"

#include <QDateTime>
#include <QObject>
#include <QSharedDataPointer>

namespace Cutelyst {
class Context;
}

/*!
 * \brief Contains data about a contact form recipient.
 */
class Recipient
{
    Q_GADGET
    Q_PROPERTY(Recipient::dbid_t id READ id CONSTANT)
    Q_PROPERTY(Form form READ form CONSTANT)
    Q_PROPERTY(QString fromName READ fromName CONSTANT)
    Q_PROPERTY(QString fromEmail READ fromEmail CONSTANT)
    Q_PROPERTY(QString toName READ toName CONSTANT)
    Q_PROPERTY(QString toEmail READ toEmail CONSTANT)
    Q_PROPERTY(QString subject READ subject CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(QString html READ html CONSTANT)
    Q_PROPERTY(QVariantMap settings READ settings CONSTANT)
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
    Q_PROPERTY(QDateTime lockedAt READ lockedAt CONSTANT)
    Q_PROPERTY(User lockedBy READ lockedBy CONSTANT)
public:
    using dbid_t = quint32;

    Recipient() noexcept = default;

    Recipient(Recipient::dbid_t id,
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
              const User &lockedBy);

    Recipient(const Recipient &other) noexcept            = default;
    Recipient(Recipient &&other) noexcept                 = default;
    Recipient &operator=(const Recipient &other) noexcept = default;
    Recipient &operator=(Recipient &&other) noexcept      = default;
    ~Recipient() noexcept                                 = default;

    void swap(Recipient &other) noexcept { data.swap(other.data); }

    [[nodiscard]] dbid_t id() const noexcept;

    [[nodiscard]] Form form() const noexcept;

    [[nodiscard]] QString fromName() const noexcept;

    [[nodiscard]] QString fromEmail() const noexcept;

    [[nodiscard]] QString toName() const noexcept;

    [[nodiscard]] QString toEmail() const noexcept;

    [[nodiscard]] QString subject() const noexcept;

    [[nodiscard]] QString text() const noexcept;

    [[nodiscard]] QString html() const noexcept;

    [[nodiscard]] QVariantMap settings() const noexcept;

    [[nodiscard]] QDateTime created() const noexcept;

    [[nodiscard]] QDateTime updated() const noexcept;

    [[nodiscard]] QDateTime lockedAt() const noexcept;

    [[nodiscard]] User lockedBy() const noexcept;

    [[nodiscard]] bool isValid() const noexcept;

    [[nodiscard]] bool isNull() const noexcept { return !data; }

    void clear()
    {
        if (!isNull()) {
            *this = Recipient();
        }
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

    static Recipient create(Cutelyst::Context *c, const Form &form, Error &e, const QVariantHash &values);

    static QList<Recipient> list(Cutelyst::Context *c, const Form &form, Error &e);

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;
        Data(Recipient::dbid_t _id,
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
             User _lockedBy);

        Data(const Data &) noexcept   = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept              = default;

        Form form;
        User lockedBy;
        QString fromName;
        QString fromEmail;
        QString toName;
        QString toEmail;
        QString subject;
        QString text;
        QString html;
        QVariantMap settings;
        QDateTime created;
        QDateTime updated;
        QDateTime lockedAt;
        Recipient::dbid_t id{0};
    };

    QSharedDataPointer<Data> data;

    friend QDataStream &operator<<(QDataStream &out, const Recipient &recipient);
    friend QDataStream &operator>>(QDataStream &in, Recipient &recipient);
};

Q_DECLARE_SHARED(Recipient) // NOLINT(modernize-type-traits)
Q_DECLARE_METATYPE(Recipient)

/*!
 * \related Form
 * \brief Writes the \a recipient to the debug stream \a dbg and returns a refererence to the stream.
 */
QDebug operator<<(QDebug dbg, const Recipient &recipient);

/*!
 * \related Form
 * \brief Writes \a recipient to stream \a out. Returns a reference to the stream.
 */
QDataStream &operator<<(QDataStream &out, const Recipient &recipient);

/*!
 * \related Form
 * \brief Reads a recipient from stream \a in into \a recipient. Returns a reference to the stream.
 */
QDataStream &operator>>(QDataStream &in, Recipient &recipient);

#endif // HBNBOTA_RECIPIENT_H
