/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "error.h"

#include <Cutelyst/Context>

#include <QSqlError>
#include <QSqlQuery>

using namespace Qt::Literals::StringLiterals;

#define HBNBOTA_ERROR_STASH_KEY u"hbnbota_error"_s

Error::Data::Data(Cutelyst::Response::HttpStatus _status, QString _text, QString _code)
    : QSharedData{}
    , status{_status}
    , text{std::move(_text)}
    , code{std::move(_code)}
{
}

Error::Data::Data(QSqlError _sqlError, QString _text)
    : QSharedData()
    , status{Cutelyst::Response::InternalServerError}
    , text{std::move(_text)}
    , code{_sqlError.nativeErrorCode()}
    , sqlError{std::move(_sqlError)}
{
}

Error::Data::Data(const QSqlQuery &_sqlQuery, QString _text)
    : Data{_sqlQuery.lastError(), std::move(_text)}
{
}

void Error::Data::setTitle(Cutelyst::Context *c)
{
    switch (status) {
    case Cutelyst::Response::BadRequest:
        //: Error title
        //% "Bad request"
        title = c->qtTrId("hbnbota_error_title_badrequest");
        break;
    case Cutelyst::Response::Unauthorized:
    case Cutelyst::Response::Forbidden:
        //: Error title
        //% "Access denied"
        title = c->qtTrId("hbnbota_error_title_accessdenied");
        break;
    case Cutelyst::Response::NotFound:
        //: Error title
        //% "Not found"
        title = c->qtTrId("hbnbota_error_title_notfound");
        break;
    case Cutelyst::Response::MethodNotAllowed:
        //: Error title
        //% "Method not allowed"
        title = c->qtTrId("hnbota_error_title_notallowed");
        break;
    default:
        //: Error title
        //% "Internal server error"
        title = c->qtTrId("hbnbota_error_title_internal");
        break;
    }
}

Cutelyst::Response::HttpStatus Error::status() const noexcept
{
    return data ? data->status : Cutelyst::Response::OK;
}

QString Error::text() const noexcept
{
    return data ? data->text : QString();
}

QString Error::sqlErrorText() const noexcept
{
    return data ? data->sqlError.text() : QString();
}

QString Error::title() const noexcept
{
    return data ? data->title : QString();
}

QString Error::code() const noexcept
{
    return data ? data->code.toUpper() : QString();
}

bool Error::isError() const noexcept
{
    return data ? data->status >= Cutelyst::Response::BadRequest : false;
}

void Error::toStash(Cutelyst::Context *c, bool detach) const
{
    Q_ASSERT(c);
    c->setStash(HBNBOTA_ERROR_STASH_KEY, QVariant::fromValue<Error>(*this));
    if (detach) {
        c->detach(c->getAction(u"error"_s));
    }
}

Error Error::fromStash(Cutelyst::Context *c)
{
    Q_ASSERT(c);
    return c->stash(HBNBOTA_ERROR_STASH_KEY).value<Error>();
}

void Error::toStash(Cutelyst::Context *c, Cutelyst::Response::HttpStatus status, const QString &text, bool detach)
{
    Error e;
    e.data = new Error::Data{status, text, {}};
    e.data->setTitle(c);
    e.toStash(c, detach);
}

QJsonObject Error::toJson() const
{
    return QJsonObject{
        {u"error"_s,
         QJsonObject{
             {u"status"_s, static_cast<int>(status())}, {u"text"_s, text()}, {u"title"_s, title()}, {u"code"_s, code()}}}};
}

bool Error::hasError(Cutelyst::Context *c) noexcept
{
    Q_ASSERT(c);
    return c->stash().contains(HBNBOTA_ERROR_STASH_KEY);
}

bool Error::operator==(const Error &other) const noexcept
{
    if (data == other.data) {
        return true;
    }

    if (status() != other.status()) {
        return false;
    }

    if (text() != other.text()) {
        return false;
    }

    if (sqlErrorText() != other.sqlErrorText()) {
        return false;
    }

    if (code() != other.code()) {
        return false;
    }

    return true;
}

Error Error::create(Cutelyst::Context *c, Cutelyst::Response::HttpStatus status, const QString &text, const QString &code)
{
    Error e;
    e.data = new Error::Data{status, text, code};
    e.data->setTitle(c);
    return e;
}

Error Error::create(Cutelyst::Context *c, const QSqlError &sqlError, const QString &text)
{
    Error e;
    e.data = new Error::Data{sqlError, text};
    e.data->setTitle(c);
    return e;
}

Error Error::create(Cutelyst::Context *c, const QSqlQuery &sqlQuery, const QString &text)
{
    Error e;
    e.data = new Error::Data{sqlQuery, text};
    e.data->setTitle(c);
    return e;
}

void swap(Error &a, Error &b) noexcept
{
    a.swap(b);
}
