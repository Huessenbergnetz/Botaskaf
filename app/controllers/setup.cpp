/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "setup.h"

#include "objects/error.h"
#include "objects/user.h"
#include "settings.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Session/session.h>
#include <Cutelyst/Plugins/Utils/Validator>
#include <Cutelyst/Plugins/Utils/validatoralphadash.h>
#include <Cutelyst/Plugins/Utils/validatorconfirmed.h>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <Cutelyst/Plugins/Utils/validatorin.h>
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <Cutelyst/Response>
#include <CutelystForms/forms.h>

using namespace Qt::Literals::StringLiterals;

Setup::Setup(QObject *parent)
    : Controller(parent)
{
}

void Setup::index(Context *c)
{
    ValidatorResult vr;
    if (c->req()->isPost()) {
        static Validator v({new ValidatorRequired(u"setuptoken"_s), new ValidatorAlphaDash(u"setuptoken"_s, true)});

        vr = v.validate(c, Validator::FillStashOnError | Validator::BodyParamsOnly);
        if (vr) {
            if (vr.value(u"setuptoken"_s).toString() == Settings::setupToken()) {
                Session::setValue(c, u"setuptoken"_s, Settings::setupToken().toLatin1());
                c->res()->redirect(c->uriFor(u"/setup"_s));
                return;
            } else {
                //: Error message
                //% "Sorry, but the entered setup token is not valid."
                vr.addError(u"setuptoken"_s, c->qtTrId("hbnbota_error_setup_invalid_token"));
            }
        }
    }

    auto form = CutelystForms::Forms::getForm(u"setup/index.qml"_s, c);
    if (!vr) {
        form->setErrors(vr.errors());
    }

    c->stash({{u"template"_s, u"setup/index.html"_s},
              //: Site third level title, will be something like: Welcome - Setup - Botaskaf
              //% "Welcome"
              {u"site_subtitle"_s, c->qtTrId("hbnbota_site_subtitle_setup_index")},
              {u"form"_s, QVariant::fromValue<CutelystForms::Form *>(form)}});
}

void Setup::defaultPage(Context *c)
{
    c->response()->body() = "Page not found!";
    c->response()->setStatus(Response::NotFound);
}

void Setup::setup(Context *c)
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
                            new ValidatorIn(u"timezone"_s, Settings::allowedTimeZoneIds())});
        vr = v.validate(c, Validator::FillStashOnError | Validator::BodyParamsOnly);
        if (vr) {
            vr.addValue(u"type"_s, static_cast<int>(User::SuperUser));
            Error e;
            auto user = User::create(c, e, vr.values());
            if (user.isValid()) {
                Session::setValue(c, u"created_user_id"_s, user.id());
                c->res()->redirect(c->uriFor(u"/finished"_s));
                return;
            } else {
                e.toStash(c);
            }
        }
    }

    const QLocale inputLocale =
        c->req()->isPost() ? QLocale{c->req()->bodyParam(u"locale"_s).trimmed()} : Settings::defLocale();
    const QTimeZone inputTz =
        c->req()->isPost() ? QTimeZone{c->req()->bodyParam(u"timezone"_s).trimmed().toLatin1()} : Settings::defTimeZone();
    auto form = CutelystForms::Forms::getForm(u"setup/setup.qml"_s, c, CutelystForms::Forms::DoNotFillContext);
    form->fieldByName(u"locale"_s)
        ->appendOptions(Settings::supportedLocales(c, inputLocale != QLocale::c() ? inputLocale : Settings::defLocale(), c));
    form->fieldByName(u"timezone"_s)
        ->appendOptions(Settings::supportedTimeZones(inputTz.isValid() ? inputTz.id() : Settings::defTimeZone().id()));
    if (!vr || Error::hasError(c)) {
        form->setErrors(vr.errors());
        form->setValues(c->req()->bodyParameters());
    }

    c->stash({{u"template"_s, u"setup/setup.html"_s},
              //: Site third level title, will be something like: Create User - Setup - Botaskaf
              //% "Create User"
              {u"site_subtitle"_s, c->qtTrId("hbnbota_site_subtitle_setup_user")},
              {u"form"_s, QVariant::fromValue<CutelystForms::Form *>(form)}});
}

void Setup::finished(Context *c)
{
    Error e;
    User u = User::get(c, e, User::toDbId(Session::value(c, u"created_user_id"_s)));
    c->stash({{u"template"_s, u"setup/finished.html"_s},
              //: Site third level title, will be something like: Finished - Setup - Botaskaf
              //% "Finished"
              {u"site_subtitle"_s, c->qtTrId("hbnbota_setup_finished_subtitle")},
              //: Message shown to the user when the setup has been finished successfully
              //% "You have successfully completed the setup and created your first user "
              //% "“%1” (%2). Now remove the “setuptoken“ entry from your configuration file "
              //% "and restart the application."
              {u"setup_finished_msg"_s, c->qtTrId("hbnbota_info_setup_finished").arg(u.displayName(), u.email())}});
}

bool Setup::Auto(Context *c)
{
    if (Session::value(c, u"created_user_id"_s).isValid() && c->actionName() != "finished"_L1) {
        c->res()->redirect(c->uriFor(u"/finished"_s));
        return false;
    }

    c->stash({{u"no_wrapper"_s, true},
              {u"site_name"_s, Settings::siteName()},
              //: Site second level title, will be something like: Welcome - Setup - Botaskaf
              //% "Setup"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_setup")}});

    if (c->actionName() == "index"_L1) {
        return true;
    }

    if (Session::value(c, u"setuptoken"_s).toString() != Settings::setupToken()) {
        c->res()->redirect(c->uriFor(u"/"_s));
        return false;
    }

    return true;
}

#include "moc_setup.cpp"
