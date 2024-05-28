// SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
// SPDX-License-Identifier: AGPL-3.0-or-later
import de.huessenbergnetz.cutelystforms 1.0

Form {
    htmlId: "addRecipient"
    method: Form.Post
    autocomplete: false

    TextForm {
        htmlId: "fromName"
        name: "fromName"
        //: Form field label
        //% "From name"
        label: cTrId("hbnbota_form_recipient_fromName_label")
    }

    TextForm {
        htmlId: "fromEmail"
        name: "fromEmail"
        required: true
        //: From field label
        //% "From email"
        label: cTrId("hbnbota_form_recipient_fromEmail_label")
    }

    TextForm {
        htmlId: "toName"
        name: "toName"
        //: Form field label
        //% "To name"
        label: cTrId("hbnbota_form_recipient_toName_label")
    }

    TextForm {
        htmlId: "toEmail"
        name: "toEmail"
        required: true
        //: Form field label
        //% "To email"
        label: cTrId("hbnbota_form_recipient_toEmail_label")
    }

    TextForm {
        htmlId: "replyToName"
        name: "replyToName"
        //: Form field label
        //% "Reply to name"
        label: cTrId("hbnbota_form_recipient_replyToName_label")
    }

    TextForm {
        htmlId: "replyToEmail"
        name: "replyToEmail"
        //: Form field label
        //% "Reply to email"
        label: cTrId("hbnbota_form_recipient_replyToEmail_label")
    }

    TextForm {
        htmlId: "subject"
        name: "subject"
        required: true
        //: Form field label
        //% "Subject"
        label: cTrId("hbnbota_form_recipient_subject_label")
    }

    TextAreaForm {
        htmlId: "text"
        name: "text"
        //: Form field label
        //% "Email content in text form"
        label: cTrId("hbnbota_form_recipient_text_label")
    }

    TextAreaForm {
        htmlId: "html"
        name: "html"
        //: Form field label
        //% "Email content in HTML form"
        label: cTrId("hbnbota_form_recipient_html_label")
    }

    buttons: [
        FormButton {
            htmlId: "cancelBtn"
            text: cTrId("hbnbota_general_cancel")
        },
        FormButton {
            htmlId: "submitBtn"
            text: cTrId("hbnbota_general_create")
        }
    ]
}
