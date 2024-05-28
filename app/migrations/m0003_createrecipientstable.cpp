/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "m0003_createrecipientstable.h"

using namespace Qt::Literals::StringLiterals;

M0003_CreateRecipientsTable::M0003_CreateRecipientsTable(Firfuorida::Migrator *parent)
    : Firfuorida::Migration{parent}
{
}

void M0003_CreateRecipientsTable::up()
{
    auto t = create(u"recipients"_s);
    t->increments();
    t->integer(u"formId"_s)->unSigned()->nullable();
    t->varChar(u"fromName"_s)->nullable()->defaultValue(u"NULL"_s);
    t->varChar(u"fromEmail"_s)->nullable()->defaultValue(u"NULL"_s);
    t->varChar(u"toName"_s)->nullable()->defaultValue(u"NULL"_s);
    t->varChar(u"toEmail"_s);
    t->varChar(u"subject"_s);
    t->text(u"text"_s)->nullable()->defaultValue(u"NULL"_s);
    t->text(u"html"_s)->nullable()->defaultValue(u"NULL"_s);
    t->json(u"settings"_s);
    t->dateTime(u"created"_s);
    t->dateTime(u"updated"_s)->nullable()->defaultValue(u"NULL"_s);
    t->dateTime(u"lockedAt"_s)->nullable()->defaultValue(u"NULL"_s);
    t->integer(u"lockedBy"_s)->nullable()->defaultValue(u"NULL"_s);
    t->foreignKey(u"formId"_s, u"forms"_s, u"id"_s, u"recipients_formId_idx"_s)
        ->onDelete(u"CASCADE"_s)
        ->onUpdate(u"CASCADE"_s);
    t->uniqueKey(QStringList({u"formId"_s, u"toEmail"_s}), u"recipients_formId_toEmail_idx"_s);

    switch (dbType()) {
    case Firfuorida::Migrator::MariaDB:
    case Firfuorida::Migrator::MySQL:
        t->setEngine(u"InnoDB"_s);
        t->setCharset(u"utf8mb4"_s);
        break;
    case Firfuorida::Migrator::DB2:
    case Firfuorida::Migrator::InterBase:
    case Firfuorida::Migrator::ODBC:
    case Firfuorida::Migrator::OCI:
    case Firfuorida::Migrator::PSQL:
    case Firfuorida::Migrator::SQLite:
    case Firfuorida::Migrator::Invalid:
        break;
    }
}

void M0003_CreateRecipientsTable::down()
{
    drop(u"recipients"_s);
}

#include "moc_m0003_createrecipientstable.cpp"
