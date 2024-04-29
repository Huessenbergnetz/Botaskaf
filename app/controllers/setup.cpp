/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "setup.h"

#include "logging.h"
#include "settings.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Validator>
#include <Cutelyst/Plugins/Utils/validatoralphadash.h>
#include <Cutelyst/Plugins/Utils/validatorconfirmed.h>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <Cutelyst/Plugins/Utils/validatorin.h>
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <Cutelyst/Response>
#include <CutelystForms/forms.h>

#include <QNetworkCookie>

using namespace Qt::Literals::StringLiterals;

Setup::Setup(QObject *parent)
    : Controller(parent)
{
}

void Setup::index(Context *c)
{
    ValidatorResult vr;
    if (c->req()->isPost()) {
        static Validator v({new ValidatorRequired(u"setuptoken"_s),
                            new ValidatorAlphaDash(u"setuptoken"_s, true)});

        vr = v.validate(c, Validator::FillStashOnError | Validator::BodyParamsOnly);
        if (vr) {
            if (vr.value(u"setuptoken"_s).toString() == Settings::setupToken()) {
                QNetworkCookie cookie{"hbnbota_setuptoken"_ba, Settings::setupToken().toLatin1()};
                cookie.setHttpOnly(true);
                cookie.setExpirationDate(
                    QDateTime::currentDateTimeUtc().addDuration(std::chrono::minutes{30}));
                c->res()->setCookie(cookie);
                c->res()->redirect(c->uriFor(u"/setup"_s));
                return;
            } else {
                //% "Sorry, but the entered setup token is not valid."
                vr.addError(u"setuptoken"_s, c->qtTrId("hbnbota_setup_invalid_token"));
            }
        }
    }

    auto form = CutelystForms::Forms::getForm(u"setup/index.qml"_s, c);
    if (!vr) {
        form->setErrors(vr.errors());
    }

    c->stash({{u"template"_s, u"setup/index.html"_s},
              //% "Welcome"
              {u"site_subtitle"_s, c->qtTrId("hbnbota_setup_index_subtitle")},
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
                            new ValidatorIn(u"locale"_s, Settings::allowedLocaleIds()),
                            new ValidatorIn(u"timezonne"_s, Settings::allowedTimeZoneIds())});
    }

    const QLocale inputLocale = c->req()->isPost()
                                    ? QLocale{c->req()->bodyParam(u"locale"_s).trimmed()}
                                    : Settings::defLocale();
    const QTimeZone inputTz =
        c->req()->isPost() ? QTimeZone{c->req()->bodyParam(u"timezone"_s).trimmed().toLatin1()}
                           : Settings::defTimeZone();
    auto form = CutelystForms::Forms::getForm(
        u"setup/setup.qml"_s, c, CutelystForms::Forms::DoNotFillContext);
    form->fieldByName(u"locale"_s)
        ->appendOptions(Settings::supportedLocales(
            c, inputLocale != QLocale::c() ? inputLocale : Settings::defLocale(), c));
    form->fieldByName(u"timezone"_s)
        ->appendOptions(Settings::supportedTimeZones(
            inputTz.isValid() ? inputTz.id() : Settings::defTimeZone().id()));
    if (!vr) {
        form->setErrors(vr.errors());
    }

    c->stash({{u"template"_s, u"setup/setup.html"_s},
              //% "Create User"
              {u"site_subtitle"_s, c->qtTrId("hbnbota_setup_user_suptitle")},
              {u"form"_s, QVariant::fromValue<CutelystForms::Form *>(form)}});
}

bool Setup::Auto(Context *c)
{
    c->stash({{u"no_wrapper"_s, true},
              {u"site_name"_s, Settings::siteName()},
              //% "Setup"
              {u"site_title"_s, c->qtTrId("hbnbota_setup_site_title")}});

    if (c->actionName() == "index"_L1) {
        return true;
    }

    if (c->req()->cookie("hbnbota_setuptoken"_ba) != Settings::setupToken().toLatin1()) {
        c->res()->redirect(c->uriFor(u"/"_s));
        return false;
    }

    return true;
}

#include "moc_setup.cpp"
