/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_ERROR_H
#define HBNBOTA_ERROR_H

#include <Cutelyst/Response>

#include <QJsonObject>
#include <QSharedDataPointer>
#include <QSqlError>

class QSqlQuery;

/*!
 * \brief Contains error data.
 */
class Error
{
public:
    /*!
     * \brief Constructs a non-error.
     *
     * isError() will return \c false and status() will return Cutelyst::Response::Ok.
     */
    Error() noexcept = default;
    /*!
     * \brief Constructs a new error with the given parameters.
     * \param status    The HTTP response status for this error.
     * \param text      The text show for this error.
     * \param code      The error code.
     */
    Error(Cutelyst::Response::HttpStatus status,
          const QString &text,
          const QString &code = QString());
    /*!
     * \brief Constructs a new error from \a sqlError with error \a text.
     */
    Error(const QSqlError &sqlError, const QString &text);
    /*!
     * \brief Constructs a new error from \a sqlQuery with error \a text.
     */
    Error(const QSqlQuery &sqlQuery, const QString &text);
    /*!
     * \brief Constructs a copy of \a other.
     */
    Error(const Error &other) noexcept = default;
    /*!
     * \brief Move-constructs an %Error instance, making it point at the same object that \a other
     * was pointing to.
     */
    Error(Error &&other) noexcept = default;
    /*!
     * \brief Assings \a other to this error and returns a reference to this error.
     */
    Error &operator=(const Error &other) noexcept = default;
    /*!
     * \brief Move-assigns \a other to this %Error instance.
     */
    Error &operator=(Error &&other) noexcept = default;
    /*!
     * \brief Destroys the error object.
     */
    ~Error() noexcept = default;

    /*!
     * \brief Swaps \a other with this error. This operation is very fast and never fails.
     */
    void swap(Error &other) noexcept { data.swap(other.data); }

    /*!
     * \brief Returns the HTTP status code.
     */
    [[nodiscard]] Cutelyst::Response::HttpStatus status() const noexcept;

    /*!
     * \brief Returns the error text.
     */
    [[nodiscard]] QString text() const noexcept;

    /*!
     * \brief Returns the text of an associated SQL error.
     */
    [[nodiscard]] QString sqlErrorText() const noexcept;

    /*!
     * \brief Returns a translated error title.
     */
    [[nodiscard]] QString title(Cutelyst::Context *c) const;

    /*!
     * \brief Returns the error code.
     */
    [[nodiscard]] QString code() const noexcept;

    /*!
     * \brief Returns \c true if this is an error.
     *
     * It is an error if the status() code is above or equal to
     * Cutelyst::Response::BadRequest (400).
     */
    [[nodiscard]] bool isError() const noexcept;

    /*!
     * \brief Converts the error to a boolean value.
     *
     * Will be the value returned by isError().
     */
    explicit operator bool() const noexcept { return isError(); }

    /*!
     * \brief Puts this error to the stash of context \a c.
     *
     * The stash key will be “hbnbota_error“. If \a detach is set to \c true,
     * the request will be detached to the error function.
     */
    void toStash(Cutelyst::Context *c, bool detach = false) const;

    /*!
     * \brief Returns an error from the stash of context \a c.
     *
     * If the stash does not contain an error, the returned error
     * will be a no-error.
     */
    static Error fromStash(Cutelyst::Context *c);

    /*!
     * \brief Constructs a new error with the given parameters and adds it to the stash.
     * \param c         Pointer to the cointer context.
     * \param status    The error status.
     * \param text      The error text.
     * \param detach    If set to \c true, the request will be detached to the error function.
     */
    static void toStash(Cutelyst::Context *c,
                        Cutelyst::Response::HttpStatus status,
                        const QString &text,
                        bool detach = false);

    /*!
     * \brief Converts the error data to a JSON object.
     *
     * The pointer to the context \a c will be used for translating the title.
     *
     * The JSON object will have the following format:
     * \code
     * {
     *    "error": {
     *        "status": 500,
     *        "title": "Internal server error",
     *        "text": "There is an error in your SQL query...",
     *        "code": "SQL_QUERY_ERROR"
     *    }
     * }
     * \endcode
     */
    QJsonObject toJson(Cutelyst::Context *c) const;

    /*!
     * \brief Returns \c true if the stash of the context \a c contains an \c error key.
     */
    static bool hasError(Cutelyst::Context *c) noexcept;

    /*!
     * \brief Returns \c true if \a other is equal to this.
     */
    bool operator==(const Error &other) const noexcept;

    /*!
     * \brief Returns \c true if \a other is not equal to this.
     */
    bool operator!=(const Error &other) const noexcept { return !(*this == other); }

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;
        Data(Cutelyst::Response::HttpStatus _status, QString _text, QString _code);
        Data(QSqlError _sqlError, QString _text);
        Data(const QSqlQuery &_sqlQuery, QString _text);
        Data(const Data &) noexcept   = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept              = default;

        Cutelyst::Response::HttpStatus status{Cutelyst::Response::OK};
        QString text;
        QString code;
        QSqlError sqlError;
    };

    QSharedDataPointer<Data> data;
};

/*!
 * \relates Error
 * \brief Swaps error \a a with error \a b. This operation is very fast and never fails.
 */
void swap(Error &a, Error &b) noexcept;

Q_DECLARE_TYPEINFO(Error, Q_RELOCATABLE_TYPE); // NOLINT(modernize-type-traits)
Q_DECLARE_METATYPE(Error)

#endif // HBNBOTA_ERROR_H
