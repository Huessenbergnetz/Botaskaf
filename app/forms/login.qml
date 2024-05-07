// SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
// SPDX-License-Identifier: AGPL-3.0-or-later
import de.huessenbergnetz.cutelystforms 1.0

Form {
    //% "Login to Botaskaf"
    label: cTrId("hbnbota_form_login_label")
    method: Form.Post

    EmailForm {
        name: "email"
        required: true
        autocomplete: "email"
        label: cTrId("hbnbota_email_label")
        placeholder: cTrId("hbnbota_email_placeholder")
    }

    PasswordForm {
        name: "password"
        required: true
        autocomplete: "password"
        label: cTrId("hbnbota_password_label")
    }

    buttons: [
        FormButton {
            htmlId: "submitBtn"
            //% "Login"
            text: cTrId("hbnbota_general_login")
        }
    ]
}
