/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_ROOT_H
#define HBNBOTA_ROOT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Root : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    explicit Root(QObject *parent = nullptr);
    ~Root();

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c);

    C_ATTR(defaultPage, :Path)
    void defaultPage(Context *c);

private:
    C_ATTR(End, :ActionClass("RenderView"))
    void End(Context *c) { Q_UNUSED(c); }
};

#endif // HBNBOTA_ROOT_H