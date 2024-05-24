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

    C_ATTR(baseForm, :Chained("/") :PathPart("forms") :CaptureArgs(1))
    void baseForm(Context *c, const QString &id);

    C_ATTR(removeForm, :Chained("baseForm") :PathPart("remove") :Args(0))
    void removeForm(Context *c);

    C_ATTR(editForm, :Chained("baseForm") :PathPart("edit") :Args(0))
    void editForm(Context *c);

    C_ATTR(recipients, :Chained("baseForm") :PathPart("recipients") :Args(0))
    void recipients(Context *c);

    C_ATTR(addRecipient, :Chained("baseForm") :PathPart("recipients/add") :Args(0))
    void addRecipient(Context *c);

    C_ATTR(baseRecipient, :Chained("baseForm") :PathPart("recipients") :CaptureArgs(1))
    void baseRecipient(Context *c, const QString &id);

    C_ATTR(editRecipient, :Chained("baseRecipient") :PathPart("edit") :Args(0))
    void editRecipient(Context *c);

    C_ATTR(removeRecipient, :Chained("baseRecipient") :PathPart("remove") :Args(0))
    void removeRecipient(Context *c);

private:
    Q_DISABLE_COPY(Forms)
};

#endif // HBNBOTA_FORMS_H
