/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_USERS_H
#define HBNBOTA_USERS_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Users final : public Controller // NOLINT(cppcoreguidelines-special-member-functions)
{
    Q_OBJECT
public:
    explicit Users(QObject *parent = nullptr);
    ~Users() override = default;

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);

    C_ATTR(add, :Local :Args(0))
    void add(Context *c);

    C_ATTR(base, :Chained("/") :PathPart("users") :CaptureArgs(1))
    void base(Context *c, const QString &id);

    C_ATTR(remove, :Chained("base") :PathPart("remove") :Args(0))
    void remove(Context *c);

    C_ATTR(edit, :Local :Args(1))
    void edit(Context *c, const QString &id);

private:
    C_ATTR(Auto, :Private)
    bool Auto(Context *c);
};

#endif // HBNBOTA_USERS_H
