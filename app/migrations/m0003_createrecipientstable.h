/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef M0003_CREATERECIPIENTSTABLE_H
#define M0003_CREATERECIPIENTSTABLE_H

#include <Firfuorida/Migration>

class M0003_CreateRecipientsTable final : public Firfuorida::Migration
{
    Q_OBJECT
    Q_DISABLE_COPY(M0003_CreateRecipientsTable)
public:
    explicit M0003_CreateRecipientsTable(Firfuorida::Migrator *parent);
    ~M0003_CreateRecipientsTable() override = default;

    void up() final;
    void down() final;
};

#endif // M0003_CREATERECIPIENTSTABLE_H
