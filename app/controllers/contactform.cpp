/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "contactform.h"

#include "objects/error.h"
#include "objects/form.h"

#include <QDateTime>

using namespace Qt::Literals::StringLiterals;

ContactForm::ContactForm(QObject *parent)
    : Controller{parent}
{
}

void ContactForm::base(Context *c, const QString &uuid)
{
    Error e;
    auto f = Form::get(c, e, uuid);
    if (f.isNull()) {
        c->res()->setJsonObjectBody(e.toJson());
        c->res()->setStatus(e.status());
        return;
    }

    f.toStash(c);
}

void ContactForm::getToken(Context *c)
{
    if (Error::hasError(c)) {
        return;
    }

    auto f = Form::fromStash(c);

    const QByteArray token = f.encrypt(QDateTime::currentDateTimeUtc());
    if (token.isEmpty()) {
        //: Error message
        //% "Failed to create token."
        const Error e =
            Error::create(c, Response::InternalServerError, c->qtTrId("hbnbota_error_contactform_failed_create_token"));
        c->res()->setJsonObjectBody(e.toJson());
        c->res()->setStatus(e.status());
        return;
    }

    c->res()->setContentType("text/plain; charset=utf-8"_ba);
    c->res()->setBody(token);
}

#include "moc_contactform.cpp"
