/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "m0002_createformstable.h"

using namespace Qt::Literals::StringLiterals;

M0002_CreateFormsTable::M0002_CreateFormsTable(Firfuorida::Migrator *parent)
    : Firfuorida::Migration{parent}
{
}

void M0002_CreateFormsTable::up()
{
    auto t = create(u"forms"_s);
    t->increments();
    t->varChar(u"uuid"_s)->unique()->charset(u"ascii"_s);
    t->integer(u"userId"_s)->unSigned()->nullable();
    t->varChar(u"secret"_s);
    t->varChar(u"name"_s);
    t->varChar(u"domain"_s);
    t->text(u"description"_s)->nullable();
    t->dateTime(u"created"_s);
    t->dateTime(u"updated"_s);
    t->dateTime(u"lockedAt"_s)->nullable();
    t->integer(u"lockedBy"_s)->unSigned()->nullable();
    t->json(u"settings"_s);
    t->foreignKey(u"userId"_s, u"users"_s, u"id"_s, u"forms_userId_idx"_s)->onDelete(u"SET NULL"_s)->onUpdate(u"CASCADE"_s);

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

void M0002_CreateFormsTable::down()
{
    drop(u"forms"_s);
}

#include "moc_m0002_createformstable.cpp"
