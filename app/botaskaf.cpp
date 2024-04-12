/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "botaskaf.h"

#include "controllers/root.h"

using namespace Cutelyst;

Botaskaf::Botaskaf(QObject *parent)
    : Application(parent)
{
}

Botaskaf::~Botaskaf()
{
}

bool Botaskaf::init()
{
    new Root(this);

    return true;
}
