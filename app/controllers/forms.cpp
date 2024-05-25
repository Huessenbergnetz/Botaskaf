/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "forms.h"

#include "logging.h"
#include "objects/error.h"
#include "objects/form.h"
#include "objects/menuitem.h"
#include "settings.h"

#include <Cutelyst/Plugins/Utils/Validator>
#include <Cutelyst/Plugins/Utils/validatorbetween.h>
#include <Cutelyst/Plugins/Utils/validatorboolean.h>
#include <Cutelyst/Plugins/Utils/validatordomain.h>
#include <Cutelyst/Plugins/Utils/validatorin.h>
#include <Cutelyst/Plugins/Utils/validatorregularexpression.h>
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <Cutelyst/Plugins/Utils/validatorrequiredif.h>
#include <CutelystForms/forms.h>

using namespace Qt::Literals::StringLiterals;

Forms::Forms(QObject *parent)
    : Controller{parent}
{
}

void Forms::index(Context *c)
{
    Error e;
    const auto forms = Form::list(c, e);
    if (e) {
        e.toStash(c);
    } else {
        c->setStash(u"forms_labels"_s, QVariant::fromValue<QMap<QString, QString>>(Form::labels(c)));
    }

    MenuItemList formsMenu;
    //: Page menu entry
    //% "Add form"
    formsMenu.emplace_back(c, u"formsMenuAdd"_s, c->qtTrId("hbnbota_formsmenu_add"), u"add"_s, u"forms"_s);

    c->stash({{u"template"_s, u"forms/index.html"_s},
              {u"forms"_s, QVariant::fromValue<QList<Form>>(forms)},
              //: Site title
              //% "Contact forms"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_forms")},
              {u"forms_menu"_s, QVariant::fromValue<MenuItemList>(formsMenu)}});
}

void Forms::add(Context *c)
{
    ValidatorResult vr;
    if (c->req()->isPost()) {
        static QRegularExpression fieldNameRegEx{uR"(^[A-Za-z][A-Za-z0-9_:.-]*$)"_s};
        static Validator v(
            {new ValidatorRequired(u"name"_s),
             new ValidatorRequired(u"domain"_s),
             new ValidatorDomain(u"domain"_s),
             new ValidatorRegularExpression(u"formFieldSenderName"_s, fieldNameRegEx),
             new ValidatorBoolean(u"formFieldSenderNameRequired"_s),
             new ValidatorRegularExpression(u"formFieldSenderEmail"_s, fieldNameRegEx),
             new ValidatorBoolean(u"formFieldSenderEmailRequired"_s),
             new ValidatorRegularExpression(u"formFieldSenderPhone"_s, fieldNameRegEx),
             new ValidatorBoolean(u"formFieldSenderPhoneRequired"_s),
             new ValidatorRegularExpression(u"formFieldSenderUrl"_s, fieldNameRegEx),
             new ValidatorBoolean(u"formFieldSenderUrlRequired"_s),
             new ValidatorRegularExpression(u"formFieldSubject"_s, fieldNameRegEx),
             new ValidatorBoolean(u"formFieldSubjectRequired"_s),
             new ValidatorRegularExpression(u"formFieldContent"_s, fieldNameRegEx),
             new ValidatorBoolean(u"formFieldContentRequired"_s),
             new ValidatorRegularExpression(u"formFieldPolicyRequired"_s, fieldNameRegEx),
             new ValidatorBoolean(u"formFieldPolicyRequired"_s),
             new ValidatorRegularExpression(u"honeypots"_s, QRegularExpression{uR"(^[A-Za-z][A-Za-z0-9_:.,-]*$)"_s}),
             new ValidatorRequired(u"senderType"_s),
             new ValidatorIn(u"senderType"_s, Settings::allowedSenderTypes()),
             new ValidatorRequiredIf(u"smtpHost"_s, u"senderType"_s, {u"SMTP"_s}),
             new ValidatorRequiredIf(u"smtpPort"_s, u"senderType"_s, {u"SMTP"_s}),
             new ValidatorBetween(u"smtpPort"_s, QMetaType::Int, 1, 65535),
             new ValidatorRequiredIf(u"smtpUser"_s, u"senderType"_s, {u"SMTP"_s}),
             new ValidatorRequiredIf(u"smtpPassword"_s, u"senderType"_s, {u"SMTP"_s}),
             new ValidatorRequiredIf(u"smtpEncryption"_s, u"senderType"_s, {u"SMTP"_s}),
             new ValidatorIn(u"smtpEncryption"_s, Settings::allowedSmtpEncryption()),
             new ValidatorRequiredIf(u"smtpAuthentication"_s, u"senderType"_s, {u"SMTP"_s}),
             new ValidatorIn(u"smtpAuthentication"_s, Settings::allowedSmtpAuthMethods())});
        vr = v.validate(c, Validator::FillStashOnError | Validator::BodyParamsOnly);
        if (vr) {
            Error e;
            auto contactForm = Form::create(c, e, vr.values());
            if (contactForm.isValid()) {
                c->res()->redirect(c->uriFor(u"/forms"_s));
                return;
            } else {
                e.toStash(c);
            }
        }
    }

    const QString senderType     = c->req()->isPost() ? c->req()->bodyParam(u"senderType"_s).trimmed() : u"SMTP"_s;
    const QString smtpEncryption = c->req()->isPost() ? c->req()->bodyParam(u"smtpEncryption"_s).trimmed() : u"TLS"_s;
    const QString smtpAuthentication =
        c->req()->isPost() ? c->req()->bodyParam(u"smtpAuthentication"_s).trimmed() : u"PLAIN"_s;

    auto form            = CutelystForms::Forms::getForm(u"forms/add.qml"_s, c, CutelystForms::Forms::DoNotFillContext);
    auto addFormSenderFs = form->fieldsetById(u"addFormSender"_s);
    addFormSenderFs->fieldById(u"senderType"_s)->appendOptions(Settings::supportedSenderTypes(c, senderType));
    addFormSenderFs->fieldById(u"smtpEncryption"_s)->appendOptions(Settings::supportedSmtpEncryption(c, smtpEncryption));
    addFormSenderFs->fieldById(u"smtpAuthentication"_s)
        ->appendOptions(Settings::supportedSmtpAuthMethods(c, smtpAuthentication));

    if (!vr || Error::hasError(c)) {
        form->setErrors(vr.errors());
        form->setValues(c->req()->bodyParameters());
        form->setAsChecked(c->req()->bodyParameters());
    }

    c->stash({{u"template"_s, u"forms/add.html"_s},
              //: Site title
              //% "Add contact form"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_add_form")},
              {u"form"_s, QVariant::fromValue<CutelystForms::Form *>(form)}});
}

void Forms::baseForm(Context *c, const QString &id)
{
    bool ok        = true;
    const auto fid = Form::toDbId(id, &ok);
    if (Q_UNLIKELY(!ok)) {
        //: Error message
        //% "The provided contact form ID is not a valid integer."
        Error::toStash(c, Response::BadRequest, c->qtTrId("hbnbota_error_invalid_form_id"), true);
        return;
    }

    Error e;
    auto form = Form::get(c, e, fid);
    if (Q_UNLIKELY(form.isNull())) {
        e.toStash(c, true);
        return;
    }

    const auto user = User::fromStash(c);
    if (!form.canEdit(user)) {
        Error::toStash(c, Response::Forbidden, c->qtTrId("hbnbota_error_general_forbidden"), true);
        qCWarning(HBNBOTA_AUTHZ) << user << "tried to access restricted area" << c->req()->uri().path();
        return;
    }

    c->setStash(u"current_form"_s, QVariant::fromValue<Form>(form));
}

void Forms::removeForm(Context *c)
{
    Q_UNUSED(c)
}

void Forms::editForm(Context *c)
{
    if (Error::hasError(c)) {
        return;
    }

    c->stash({{u"template"_s, u"forms/edit.html"_s},
              //: Site title
              //% "Edit contact form"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_edit_form")}});
}

void Forms::recipients(Context *c)
{
    Q_UNUSED(c)
}

void Forms::addRecipient(Context *c)
{
    Q_UNUSED(c)
}

void Forms::baseRecipient(Context *c, const QString &id)
{
    Q_UNUSED(c)
    Q_UNUSED(id)
}

void Forms::editRecipient(Context *c)
{
    Q_UNUSED(c)
}

void Forms::removeRecipient(Context *c)
{
    Q_UNUSED(c)
}

#include "moc_forms.cpp"
