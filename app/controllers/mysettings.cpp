/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "mysettings.h"

using namespace Qt::Literals::StringLiterals;

MySettings::MySettings(QObject *parent)
    : Controller{parent}
{
}

void MySettings::index(Context *c)
{
    c->stash({{u"template"_s, u"mysettings/index.html"_s},
              //: Site title
              //% "My settings"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_mysettings")}});
}

#include "moc_mysettings.cpp"
