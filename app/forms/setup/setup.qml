// SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
// SPDX-License-Identifier: AGPL-3.0-or-later
import de.huessenbergnetz.cutelystforms 1.0

Form {
    //: Form label, migth be used as form header
    //% "Create your admin user"
    label: cTrId("hbnbota_form_setup_setup_label")
    //: Form description
    //% "Create your first user that acts as super administrator."
    description: cTrId("hbnbota_form_setup_setup_desc")
    method: Form.Post

    EmailForm {
        htmlId: "email"
        name: "email"
        required: true
        autocomplete: "email"
        //: Form field label
        //% "Email address"
        label: cTrId("hbnbota_email_label")
        //: Form field description
        //% "The user’s email address. Will be used for login and for system notifications."
        description: cTrId("hbnbota_user_email_desc")
        //: Form field placeholder
        //% "your.name@example.com"
        placeholder: cTrId("hbnbota_email_placeholder")
        value: cStashValue("email")
    }

    TextForm {
        htmlId: "displayName"
        name: "displayName"
        required: true
        minlength: 3
        autocomplete: "name"
        //: Form field label
        //% "Display name"
        label: cTrId("hbnbota_displayname_label")
        //: Form field description
        //% "The display name will be shown instead of the email address. Feel free to use a "
        //% "real name or a nickmane."
        description: cTrId("hbnbota_user_displayname_desc")
        //: Form field placeholder
        //% "John Doe"
        placeholder: cTrId("hbnbota_displayname_placeholder")
        value: cStashValue("displayName")
    }

    PasswordForm {
        htmlId: "password"
        name: "password"
        required: true
        autocomplete: "new-password"
        //: Form field label
        //% "Password"
        label: cTrId("hbnbota_password_label")
    }

    PasswordForm {
        htmlId: "passConfirm"
        name: "password_confirmation"
        required: true
        autocomplete: "new-password"
        //: Form field label
        //% "Password confirmation"
        label: cTrId("hbnbota_password_confirmation_label")
        //: Form field description
        //% "Please confirm your selected password by enter it again."
        description: cTrId("hbnbota_password_confirmation_desc")
    }

    Select {
        htmlId: "locale"
        name: "locale"
        required: true
        //: Form field label
        //% "Localization"
        label: cTrId("hbnbota_user_locale_label")
    }

    Select {
        htmlId: "timezone"
        name: "timezone"
        required: true
        //: Form field label
        //% "Time zone"
        label: cTrId("hbnbota_user_timezone_label")
    }

    buttons: [
        FormButton {
            htmlId: "submitBtn"
            //: Form button text
            //% "Create"
            text: cTrId("hbnbota_general_create")
        }
    ]
}
