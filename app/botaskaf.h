/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_BOTASKAF_H
#define HBNBOTA_BOTASKAF_H

#include <Cutelyst/Application>

using namespace Cutelyst;

class Botaskaf : public Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "Botaskaf")
public:
    Q_INVOKABLE explicit Botaskaf(QObject *parent = nullptr);
    ~Botaskaf();

    bool init();
};

#endif // HBNBOTA_BOTASKAF_H
