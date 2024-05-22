/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "forms.h"

#include "objects/menuitem.h"
#include "settings.h"

#include <CutelystForms/forms.h>

using namespace Qt::Literals::StringLiterals;

Forms::Forms(QObject *parent)
    : Controller{parent}
{
}

void Forms::index(Context *c)
{
    MenuItemList formsMenu;
    //: Page menu entry
    //% "Add form"
    formsMenu.emplace_back(c, u"formsMenuAdd"_s, c->qtTrId("hbnbota_formsmenu_add"), u"add"_s, u"forms"_s);

    c->stash({{u"template"_s, u"forms/index.html"_s},
              //: Site title
              //% "Contact forms"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_forms")},
              {u"forms_menu"_s, QVariant::fromValue<MenuItemList>(formsMenu)}});
}

void Forms::add(Context *c)
{

    auto form            = CutelystForms::Forms::getForm(u"forms/add.qml"_s, c, CutelystForms::Forms::DoNotFillContext);
    auto addFormSenderFs = form->fieldsetById(u"addFormSender"_s);
    addFormSenderFs->fieldById(u"senderType"_s)->appendOptions(Settings::supportedSenderTypes(c, u"SMTP"_s));
    addFormSenderFs->fieldById(u"smtpEncryption"_s)->appendOptions(Settings::supportedSmtpEncryption(c, u"TLS"_s));
    addFormSenderFs->fieldById(u"smtpAuthentication"_s)->appendOptions(Settings::supportedSmtpAuthMethods(c, u"PLAIN"_s));

    c->stash({{u"template"_s, u"forms/add.html"_s},
              //: Site title
              //% "Add contact form"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_add_form")},
              {u"form"_s, QVariant::fromValue<CutelystForms::Form *>(form)}});
}

#include "moc_forms.cpp"
