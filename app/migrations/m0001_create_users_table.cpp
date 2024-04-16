/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "m0001_create_users_table.h"

using namespace Qt::Literals::StringLiterals;

M0001_CreateUsersTable::M0001_CreateUsersTable(Firfuorida::Migrator *parent)
    : Firfuorida::Migration{parent}
{
}

void M0001_CreateUsersTable::up()
{
    auto t = create(u"users"_s);
    t->increments();
    t->tinyInteger(u"type"_s)->defaultValue(0);
    t->varChar(u"email"_s)->unique();
    t->varChar(u"displayName"_s);
    t->varChar(u"password"_s)->nullable()->charset(u"ascii"_s);
    t->dateTime(u"created"_s);
    t->dateTime(u"updated"_s);
    t->dateTime(u"lastSeen"_s)->nullable();
    t->dateTime(u"lockedAt"_s)->nullable();
    t->integer(u"lockedBy"_s)->unSigned()->nullable();

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

void M0001_CreateUsersTable::down()
{
    drop(u"users"_s);
}

#include "moc_m0001_create_users_table.cpp"
