/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_CONTACTFORM_H
#define HBNBOTA_CONTACTFORM_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class ContactForm final : public Controller
{
    Q_OBJECT
    C_NAMESPACE("contactform")
public:
    explicit ContactForm(QObject *parent = nullptr);
    ~ContactForm() override = default;

    C_ATTR(base, :Chained("/") :PathPart("contactform") :CaptureArgs(1))
    void base(Context *c, const QString &uuid);

    C_ATTR(getToken, :Chained("base") :PathPart("gettoken") :Args(0))
    void getToken(Context *c);
};

#endif // HBNBOTA_CONTACTFORM_H
