/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_USER_H
#define HBNBOTA_USER_H

#include <QDateTime>
#include <QJsonObject>
#include <QObject>
#include <QSharedDataPointer>

class UserData;
class Error;

namespace Cutelyst {
class Context;
}

/*!
 * \brief Contains data about users and provides user management methods.
 */
class User
{
    Q_GADGET
    /*!
     * \brief The database ID.
     */
    Q_PROPERTY(User::dbid_t id READ id CONSTANT)
    /*!
     * \brief The user type.
     */
    Q_PROPERTY(User::Type type READ type CONSTANT)
    /*!
     * \brief The user’s email address.
     */
    Q_PROPERTY(QString email READ email CONSTANT)
    /*!
     * \brief The user’s display name.
     */
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    /*!
     * \brief UTC date and time the user has been created.
     */
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    /*!
     * \brief UTC date and time the user has been last updated.
     */
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
    /*!
     * \brief UTC date and time the user has been last seen.
     *
     * If the user has never been seen, this will be a null datetime.
     */
    Q_PROPERTY(QDateTime lastSeen READ lastSeen CONSTANT)
    /*!
     * \brief UTC date and time the user has been locked for editing.
     *
     * If the user has not been locked, this will be a null datetime.
     */
    Q_PROPERTY(QDateTime lockedAt READ lockedAt CONSTANT)
    /*!
     * \brief Database ID of the user that locked this user for editing.
     *
     * If the user has not been locked, this will be \c 0.
     */
    Q_PROPERTY(User::dbid_t lockedById READ lockedById CONSTANT)
    /*!
     * \brief Display name of the user that locked this user for editing.
     *
     * If the user has not been locked, this will be a null string.
     */
    Q_PROPERTY(QString lockedByName READ lockedByName CONSTANT)
    /*!
     * \brief Map of settings for this user.
     */
    Q_PROPERTY(QVariantMap settings READ settings CONSTANT)
    /*!
     * \brief IANA time zone ID the user uses.
     */
    Q_PROPERTY(QString timezone READ timezone CONSTANT)
    /*!
     * \brief Locale of the user in the form language_TERRITORY.
     */
    Q_PROPERTY(QString locale READ locale CONSTANT)
    /*!
     * \brief \c true if this user is an administrator, otherwise \c false.
     */
    Q_PROPERTY(bool isAdmin READ isAdmin CONSTANT)
public:
    /*!
     * \brief Database ID type.
     */
    using dbid_t = quint32;

    /*!
     * \brief The user type.
     */
    enum Type : int {
        Invalid       = -1,
        Disabled      = 0,
        Registered    = 16,
        Manager       = 32,
        Administrator = 64,
        SuperUser     = 127
    };
    Q_ENUM(Type)

    /*!
     * \brief Constructs a new \a null %User object.
     *
     * isNull() returns \c true for this, isValid() returns \c false.
     */
    User() noexcept;
    /*!
     * \brief Constructs a new %User object with the given parameters.
     * \param id            The database id.
     * \param type          The user type.
     * \param email         The email address.
     * \param displayName   The display name.
     * \param created       UTC date and time the user has been created.
     * \param updated       UTC date and time the user has been updated.
     * \param lastSeen      UTC date and time the user has been last seen.
     * \param lockedAt      UTC date and time the user has been locked for editing.
     * \param lockedById    Database id of the the user that locked this user for editing.
     * \param lockedByName  Display name of the user that locked this user for editing.
     * \param settings      Settings for this user.
     */
    User(dbid_t id,
         Type type,
         const QString &email,
         const QString &displayName,
         const QDateTime &created,
         const QDateTime &updated,
         const QDateTime &lastSeen,
         const QDateTime &lockedAt,
         dbid_t lockedById,
         const QString &lockedByName,
         const QVariantMap &settings);
    /*!
     * \brief Constructs a copy of \a other.
     */
    User(const User &other) noexcept;
    /*!
     * \brief Move-constructs a %User instance , making it point at the same object that \a other
     * was pointing to.
     */
    User(User &&other) noexcept;
    /*!
     * \brief Assigns \a other to this user and returns a reference to this user.
     */
    User &operator=(const User &other) noexcept;
    /*!
     * \brief Move-assigns \a other to this %User instance.
     */
    User &operator=(User &&other) noexcept;
    /*!
     * \brief Destroys the %User object.
     */
    ~User() noexcept;

    /*!
     * \brief Swaps user \a other with this user.
     *
     * This operation is very fast and never fails.
     */
    void swap(User &other) noexcept { data.swap(other.data); }

    /*!
     * \brief Returns the user’s database ID.
     */
    [[nodiscard]] dbid_t id() const noexcept;

    /*!
     * \brief Returns the type of this user.
     */
    [[nodiscard]] Type type() const noexcept;

    /*!
     * \brief Returns the user’s email address.
     */
    [[nodiscard]] QString email() const noexcept;

    /*!
     * \brief Returns the user’s display name.
     */
    [[nodiscard]] QString displayName() const noexcept;

    /*!
     * \brief Returns the UTC date and time the user has been created.
     */
    [[nodiscard]] QDateTime created() const noexcept;

    /*!
     * \brief Returns the UTC date and time the user has been updated.
     */
    [[nodiscard]] QDateTime updated() const noexcept;

    /*!
     * \brief Returns the UTC date and time the user has been last seen.
     *
     * If the user never logged in, this will return a \c null datetime.
     */
    [[nodiscard]] QDateTime lastSeen() const noexcept;

    /*!
     * \brief Returns the UTC date and time the user data has been locked.
     *
     * If the user has not been locked, this will return a \c null datetime.
     */
    [[nodiscard]] QDateTime lockedAt() const noexcept;

    /*!
     * \brief Returns the database id of the user that locked this user.
     *
     * If the user has not been locked, this will return \c 0.
     */
    [[nodiscard]] dbid_t lockedById() const noexcept;

    /*!
     * \brief Returns the display name of the user that locked this user.
     *
     * If the user has not been locked, this will return an empty string.
     */
    [[nodiscard]] QString lockedByName() const noexcept;

    /*!
     * \brief Returns the user’s settings.
     */
    [[nodiscard]] QVariantMap settings() const;

    /*!
     * \brief Returns a specific settings value for this user identified by \a key.
     *
     * If \a key is not present in the settings and \a defValue is given, this will
     * be returned instead. If this user object is a null object, an invalid variant
     * will be returned.
     */
    [[nodiscard]] QVariant setting(const QString &key, const QVariant &defValue = QVariant()) const;

    /*!
     * \brief Returns the time zone the user uses as IANA time zone ID.
     *
     * Defaults to \c 'UTC'. On a null user an empty string will be returned.
     */
    [[nodiscard]] QString timezone() const;

    /*!
     * \brief Returns the locale the user uses in the form of \c language_TERRITORY.
     *
     * Defaults to \c 'en_US'. On a null user an empty string will be returned.
     */
    [[nodiscard]] QString locale() const;

    /*!
     * \brief Returns \c true if this user is of type Administrator or higher.
     */
    [[nodiscard]] bool isAdmin() const noexcept;

    /*!
     * \brief Returns \c true if this user is valid.
     *
     * A user is valid if the id() is greater than \c 0 and if the type()
     * is not Invalid.
     */
    [[nodiscard]] bool isValid() const noexcept;

    /*!
     * \brief Returns \c true if this is a \c null user object.
     */
    [[nodiscard]] bool isNull() const noexcept { return !data; }

    /*!
     * \brief Clears the user object.
     *
     * After calling this, isNull() will return \c true and isValid() will
     * return \c false.
     */
    void clear()
    {
        if (!isNull())
            *this = User();
    }

    /*!
     * \brief Converts the user object into a JSON object.
     *
     * Property names will be the same as for the %User object.
     */
    [[nodiscard]] QJsonObject toJson() const;

    /*!
     * \brief Returns \c true if \a other user is equal to this user; otherwise \c false.
     *
     * This compares all properties.
     */
    bool operator==(const User &other) const noexcept;

    /*!
     * \brief Returns \c true if \a other user is not equal to this user; otherwise \c false.
     *
     * This compares all properties.
     */
    bool operator!=(const User &other) const noexcept { return !(*this == other); }

    /*!
     * \brief Converts \a str to enum Type.
     */
    static Type typeStringToEnum(QStringView str);

    /*!
     * \brief Converts enum \y type to a string.
     */
    static QString typeEnumToString(Type type);

    /*!
     * \brief Returns a string list of supported user type names.
     *
     * This will contain the key names.
     */
    static QStringList supportedTypes();

    /*!
     * \brief Returns a string list of supported user type values.
     *
     * This will contain the key values.
     */
    static QStringList typeValues();

    /*!
     * \brief Returns  a string list of supported user type values \a below a specific Type.
     *
     * This will contain the key values.
     */
    static QStringList typeValues(Type below);

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

    static User fromStash(Cutelyst::Context *c);

    void toStash(Cutelyst::Context *c) const;

    static User create(Cutelyst::Context *c, Error &e, const QVariantHash &values);

private:
    QSharedDataPointer<UserData> data;

    friend QDataStream &operator<<(QDataStream &out, const User &user);
    friend QDataStream &operator>>(QDataStream &in, User &user);

    static User fromCache(User::dbid_t id);

    void toCache() const;
};

Q_DECLARE_METATYPE(User)
Q_DECLARE_TYPEINFO(User, Q_RELOCATABLE_TYPE); // NOLINT(modernize-type-traits)

/*!
 * \related
 * \brief Writes the \a user to the debug stream \a dbg and returns a reference to the stream.
 */
QDebug operator<<(QDebug dbg, const User &user);

/*!
 * \related User
 * \brief Writes \a user to stream \a out. Returns a reference to the stream.
 */
QDataStream &operator<<(QDataStream &out, const User &user);

/*!
 * \related User
 * \brief Reds a user from stream \a in into \a user. Returns a reference to the stream.
 */
QDataStream &operator>>(QDataStream &in, User &user);

/*!
 * \related User
 * \brief Swaps user \a a with user \a b.
 *
 * This operation is very fast and never fails.
 */
void swap(User &a, User &b) noexcept;

#endif // HBNBOTA_USER_H
