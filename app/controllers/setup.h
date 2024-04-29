/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_SETUP_H
#define HBNBOTA_SETUP_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Setup final : public Controller
{
    Q_OBJECT
    Q_DISABLE_COPY(Setup)
    C_NAMESPACE("")
public:
    explicit Setup(QObject *parent = nullptr);
    ~Setup() override = default;

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c);

    C_ATTR(defaultPage, :Path)
    void defaultPage(Context *c);

    C_ATTR(setup, :Local :Args(0))
    void setup(Context *c);

    C_ATTR(finished, :Local :Args(0))
    void finished(Context *c);

private:
    C_ATTR(End, :ActionClass("RenderView"))
    void End(Context *c) { Q_UNUSED(c); }

    C_ATTR(Auto, :Private)
    bool Auto(Context *c);
};

#endif // HBNBOTA_SETUP_H
