// SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
// SPDX-License-Identifier: AGPL-3.0-or-later
import de.huessenbergnetz.cutelystforms 1.0

Form {
    htmlId: "addUserForm"
    method: Form.Post

    EmailForm {
        htmlId: "email"
        name: "email"
        required: true
        autocomplete: "email"
        label: cTrId("hbnbota_email_label")
        description: cTrId("hbnbota_user_email_desc")
        placeholder: cTrId("hbnbota_email_placeholder")
    }

    TextForm {
        htmlId: "displayName"
        name: "displayName"
        required: true
        minlength: 3
        autocomplete: "name"
        label: cTrId("hbnbota_displayname_label")
        description: cTrId("hbnbota_user_displayname_desc")
        placeholder: cTrId("hbnbota_displayname_placeholder")
    }

    PasswordForm {
        htmlId: "password"
        name: "password"
        required: true
        autocomplete: "new-password"
        label: cTrId("hbnbota_password_label")
    }

    PasswordForm {
        htmlId: "passConfirm"
        name: "password_confirmation"
        required: true
        autocomplete: "new-password"
        label: cTrId("hbnbota_password_confirmation_label")
        description: cTrId("hbnbota_password_confirmation_desc")
    }

    Select {
        htmlId: "locale"
        name: "locale"
        required: true
        label: cTrId("hbnbota_user_locale_label")
    }

    Select {
        htmlId: "timezone"
        name: "timezone"
        required: true
        label: cTrId("hbnbota_user_timezone_label")
    }

    Select {
        htmlId: "type"
        name: "type"
        required: true
        //% "Type"
        label: cTrId("hbnbota_user_type_label")
    }

    buttons: [
        FormButton {
            htmlId: "cancelBtn"
            //% "Cancel"
            text: cTrId("hbnbota_general_cancel")
        },
        FormButton {
            htmlId: "submitBtn"
            text: cTrId("hbnbota_general_create")
        }
    ]
}
