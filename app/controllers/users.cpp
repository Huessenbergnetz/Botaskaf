/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "users.h"

#include "logging.h"
#include "objects/error.h"
#include "objects/menuitem.h"
#include "objects/user.h"
#include "settings.h"

#include <Cutelyst/Plugins/Utils/Validator>
#include <Cutelyst/Plugins/Utils/validatorconfirmed.h>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <Cutelyst/Plugins/Utils/validatorin.h>
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <CutelystForms/forms.h>

using namespace Qt::Literals::StringLiterals;

Users::Users(QObject *parent)
    : Controller(parent)
{
}

void Users::index(Context *c)
{
    Error e;
    const auto users = User::list(c, e);
    if (e) {
        e.toStash(c);
    } else {
        c->setStash(u"users_labels"_s, QVariant::fromValue<QMap<QString, QString>>(User::labels(c)));
    }

    MenuItemList usersMenu;
    //: Page menu entry
    //% "Add user"
    usersMenu.emplaceBack(c, u"usersMenuAdd"_s, c->qtTrId("hbnbota_usersmenu_add"), u"add"_s, u"users"_s);

    c->stash({{u"template"_s, u"users/index.html"_s},
              {u"users"_s, QVariant::fromValue<QList<User>>(users)},
              //: Site title
              //% "Users"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_users")},
              {u"users_menu"_s, QVariant::fromValue<MenuItemList>(usersMenu)}});
}

void Users::add(Context *c)
{
    ValidatorResult vr;
    if (c->req()->isPost()) {
        static Validator v({new ValidatorRequired(u"email"_s),
                            new ValidatorEmail(u"email"_s),
                            new ValidatorRequired(u"displayName"_s),
                            new ValidatorRequired(u"password"_s),
                            new ValidatorConfirmed(u"password"_s),
                            new ValidatorRequired(u"locale"_s),
                            new ValidatorIn(u"locale"_s, Settings::allowedLocaleIds()),
                            new ValidatorRequired(u"timezone"_s),
                            new ValidatorIn(u"timezone"_s, Settings::allowedTimeZoneIds()),
                            new ValidatorRequired(u"type"_s),
                            new ValidatorIn(u"type"_s, User::typeValues())});
        vr = v.validate(c, Validator::FillStashOnError | Validator::BodyParamsOnly);
        if (vr) {
            Error e;
            auto user = User::create(c, e, vr.values());
            if (user.isValid()) {
                c->res()->redirect(c->uriFor(u"/users"_s));
                return;
            } else {
                e.toStash(c);
            }
        }
    }

    // setup form
    const QLocale inputLocale =
        c->req()->isPost() ? QLocale{c->req()->bodyParam(u"locale"_s).trimmed()} : Settings::defLocale();
    const QTimeZone inputTz =
        c->req()->isPost() ? QTimeZone{c->req()->bodyParam(u"timezone"_s).trimmed().toLatin1()} : Settings::defTimeZone();
    const User::Type inputType =
        c->req()->isPost() ? static_cast<User::Type>(c->req()->bodyParam(u"type"_s).toInt()) : User::Type::Registered;

    auto form = CutelystForms::Forms::getForm(u"users/add.qml"_s, c, CutelystForms::Forms::DoNotFillContext);
    form->fieldByName(u"locale"_s)
        ->appendOptions(Settings::supportedLocales(c, inputLocale != QLocale::c() ? inputLocale : Settings::defLocale()));
    form->fieldByName(u"timezone"_s)
        ->appendOptions(Settings::supportedTimeZones(inputTz.isValid() ? inputTz.id() : Settings::defTimeZone().id()));
    form->fieldByName(u"type"_s)->appendOptions(
        User::supportedTypes(inputType != User::Invalid ? inputType : User::Type::Registered));

    if (!vr || Error::hasError(c)) {
        form->setErrors(vr.errors());
        form->setValues(c->req()->bodyParameters());
    }

    c->stash({{u"template"_s, u"users/add.html"_s},
              //: Site title
              //% "Add user"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_add_user")},
              {u"form"_s, QVariant::fromValue<CutelystForms::Form *>(form)}});
}

void Users::edit(Context *c, const QString &id)
{
    bool ok        = true;
    const auto uid = User::toDbId(id, &ok);
    if (Q_UNLIKELY(!ok)) {
        //: Error message
        //% "The provided user ID is not a valid integer."
        Error::toStash(c, Response::BadRequest, c->qtTrId("hbnbota_error_invalid_user_id"), true);
        return;
    }

    Error e;
    auto user = User::get(c, e, uid);
    if (Q_UNLIKELY(user.isNull())) {
        e.toStash(c, true);
        return;
    }

    c->stash({{u"template"_s, u"users/edit.html"_s},
              //: Site title
              //% "Edit user"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_edit_user")}});
}

bool Users::Auto(Context *c)
{
    if (User::fromStash(c).type() < User::Administrator) {
        //: Error message
        //% "Sorry, but you do not have access authorization for this area."
        Error::toStash(c, Response::Forbidden, c->qtTrId("hbnbota_error_general_forbidden"), true);
        qCWarning(HBNBOTA_AUTHZ) << User::fromStash(c) << "tried to access restricted area" << c->req()->uri().path();
        return false;
    }

    return true;
}

#include "moc_users.cpp"
