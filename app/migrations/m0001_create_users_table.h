/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef M0001_CREATE_USERS_TABLE_H
#define M0001_CREATE_USERS_TABLE_H

#include <Firfuorida/Migration>

class M0001_CreateUsersTable final : public Firfuorida::Migration
{
    Q_OBJECT
    Q_DISABLE_COPY(M0001_CreateUsersTable)
public:
    explicit M0001_CreateUsersTable(Firfuorida::Migrator *parent);
    ~M0001_CreateUsersTable() override = default;

    void up() final;
    void down() final;
};

#endif // M0001_CREATE_USERS_TABLE_H
