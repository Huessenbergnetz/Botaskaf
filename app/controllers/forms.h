/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_FORMS_H
#define HBNBOTA_FORMS_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Forms final : public Controller // NOLINT(cppcoreguidelines-special-member-functions)
{
    Q_OBJECT
public:
    explicit Forms(QObject *parent = nullptr);
    ~Forms() override = default;

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);

    C_ATTR(add, :Local :Args(0))
    void add(Context *c);

private:
    Q_DISABLE_COPY(Forms)
};

#endif // HBNBOTA_FORMS_H
