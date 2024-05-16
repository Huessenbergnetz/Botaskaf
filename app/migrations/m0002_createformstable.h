/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef M0002_CREATEFORMSTABLE_H
#define M0002_CREATEFORMSTABLE_H

#include <Firfuorida/Migration>

class M0002_CreateFormsTable final : public Firfuorida::Migration
{
    Q_OBJECT
    Q_DISABLE_COPY(M0002_CreateFormsTable)
public:
    explicit M0002_CreateFormsTable(Firfuorida::Migrator *parent);
    ~M0002_CreateFormsTable() override = default;

    void up() final;
    void down() final;
};

#endif // M0002_CREATEFORMSTABLE_H
