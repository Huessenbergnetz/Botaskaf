/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "recipientlist.h"

#include <Cutelyst/Context>

using namespace Qt::Literals::StringLiterals;

RecipientList::Data::Data(const QList<Recipient> &_entries)
    : QSharedData()
    , entries{_entries}
{
}

RecipientList::Data::Data(QList<Recipient> &&_entries)
    : QSharedData()
    , entries{std::move(_entries)}
{
}

RecipientList::Data::Data(const Error &_error)
    : QSharedData()
    , error{_error}
{
}

RecipientList::Data::Data(Error &&_error)
    : QSharedData()
    , error{std::move(_error)}
{
}

void RecipientList::Data::loadLabels(Cutelyst::Context *c)
{
    labels = Recipient::labels(c);
}

RecipientList::RecipientList(const QList<Recipient> &recipients)
    : data{new Data{recipients}}
{
}

RecipientList::RecipientList(QList<Recipient> &&recipients)
    : data{new Data{std::move(recipients)}}
{
}

RecipientList::RecipientList(const Error &error)
    : data{new Data{error}}
{
}

RecipientList::RecipientList(Error &&error)
    : data{new Data{std::move(error)}}
{
}

QList<Recipient> RecipientList::entries() const noexcept
{
    return data ? data->entries : QList<Recipient>();
}

int RecipientList::size() const noexcept
{
    return data ? static_cast<int>(data->entries.size()) : 0;
}

Error RecipientList::error() const noexcept
{
    return data ? data->error : Error();
}

bool RecipientList::hasError() const noexcept
{
    return data ? data->error.isError() : false;
}

QMap<QString, QString> RecipientList::labels() const noexcept
{
    return data ? data->labels : QMap<QString, QString>();
}

bool RecipientList::isEmpty() const noexcept
{
    return data ? data->entries.empty() : true;
}

RecipientList RecipientList::get(Cutelyst::Context *c, const Form &form)
{
    RecipientList rl;
    Error e;
    const auto lst = Recipient::list(c, form, e);
    rl             = e ? RecipientList{e} : RecipientList{lst};
    rl.data->loadLabels(c);
    return rl;
}

#include "moc_recipientlist.cpp"
