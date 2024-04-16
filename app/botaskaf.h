/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_BOTASKAF_H
#define HBNBOTA_BOTASKAF_H

#include <Cutelyst/Application>

using namespace Cutelyst;

class Botaskaf final : public Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "Botaskaf")
    Q_DISABLE_COPY(Botaskaf)
public:
    Q_INVOKABLE explicit Botaskaf(QObject *parent = nullptr);
    ~Botaskaf() override = default;

    bool init() override;

    bool postFork() override;

private:
    [[nodiscard]] bool connectDb(const QString &conName = QString()) const;
    [[nodiscard]] bool initializeDb(const QString &conName) const;
};

#endif // HBNBOTA_BOTASKAF_H
