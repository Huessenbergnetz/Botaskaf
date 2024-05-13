/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "login.h"

#include "logging.h"
#include "objects/error.h"
#include "objects/user.h"
#include "qtimezonevariant_p.h"
#include "settings.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Utils/Validator>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <CutelystForms/forms.h>

using namespace Qt::Literals::StringLiterals;

Login::Login(QObject *parent)
    : Controller(parent)
{
}

void Login::index(Context *c)
{
    ValidatorResult vr;
    if (c->req()->isPost()) {
        static Validator v({new ValidatorRequired(u"email"_s),
                            new ValidatorEmail(u"email"_s, ValidatorEmail::RFC5321, ValidatorEmail::AllowIDN),
                            new ValidatorRequired(u"password"_s)});

        vr = v.validate(c, Validator::BodyParamsOnly);

        if (vr) {
            const auto values = vr.values();
            ParamsMultiMap data;
            for (auto i = values.cbegin(), end = values.cend(); i != end; i++) {
                data.insert(i.key(), i.value().toString());
            }

            if (Authentication::authenticate(c, data)) {

                auto authUser = Authentication::user(c);
                Error e;
                const auto u = User::get(c, e, authUser.id().toUInt());

                if (Q_LIKELY(!u.isNull())) {
                    u.toStash(c);

                    QLocale locale{u.locale()};
                    if (locale.language() == QLocale::C) {
                        qCWarning(HBNBOTA_CORE) << "Invalid locale" << u.locale() << "selected by" << u << "Falling back to"
                                                << Settings::defLocale();
                        locale = Settings::defLocale();
                    }
                    Session::setValue(c, u"locale"_s, QVariant::fromValue<QLocale>(locale));

                    QTimeZone tz{u.timezone().toLatin1()};
                    if (!tz.isValid()) {
                        qCWarning(HBNBOTA_CORE) << "Invalid time zone" << u.timezone() << "selected by" << u
                                                << "Falling back to" << Settings::defTimeZone();
                        tz = Settings::defTimeZone();
                    }
                    Session::setValue(c, u"tz"_s, QVariant::fromValue<QTimeZone>(tz));
                }

                qCInfo(HBNBOTA_AUTHN) << u << "successfully logged in";

                c->res()->redirect(c->uriFor(u"/"_s));
                return;
            } else {
                if (!Error::hasError(c)) {
                    qCWarning(HBNBOTA_AUTHN).noquote().nospace()
                        << "Login failed: bad password for user identified by " << vr.value(u"email"_s).toString()
                        << ". IP: " << c->req()->addressString();
                    const Error e =
                        Error::create(c, Response::Forbidden, c->qtTrId("hbnbota_error_login_failed"), u"BAD_LOGIN"_s);
                    e.toStash(c);
                }
                c->res()->setStatus(Error::fromStash(c).status());
            }
        }
    }

    auto form = CutelystForms::Forms::getForm(u"login.qml"_s, c, CutelystForms::Forms::DoNotFillContext);
    form->fieldByName(u"email"_s)->setValue(c->req()->bodyParameter(u"email"_s));
    if (!vr) {
        form->setErrors(vr.errors());
    }

    c->stash({{u"template"_s, u"login.html"_s},
              {u"no_wrapper"_s, true},
              //: Site second level title, will be something like: Login - Botaskaf
              //% "Login"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_login")},
              {u"site_name"_s, Settings::siteName()},
              {u"form"_s, QVariant::fromValue<CutelystForms::Form *>(form)}});
}

#include "moc_login.cpp"
