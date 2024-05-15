/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_MYSETTINGS_H
#define HBNBOTA_MYSETTINGS_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class MySettings final : public Controller
{
    Q_OBJECT
    C_NAMESPACE("mysettings")
public:
    explicit MySettings(QObject *parent = nullptr);
    ~MySettings() override = default;

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c);

private:
    Q_DISABLE_COPY(MySettings)
};

#endif // HBNBOTA_MYSETTINGS_H
