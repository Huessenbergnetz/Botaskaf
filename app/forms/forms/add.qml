// SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
// SPDX-License-Identifier: AGPL-3.0-or-later
import de.huessenbergnetz.cutelystforms 1.0

Form {
    htmlId: "addForm"
    method: Form.Post
    autocomplete: false

    fieldsets: [
        Fieldset {
            htmlId: "addFormGeneral"
            //: Fieldset legend
            //% "General settings"
            legend: cTrId("hbnbota_form_general_fieldset_legend")

            TextForm {
                htmlId: "name"
                name: "name"
                required: true
                minlength: 3
                //: Form field label, name of the contact form
                //% "Name"
                label: cTrId("hbnbota_form_name_label")
                //: Form field description
                //% "The name for the contact form is used internally as a hint for the user."
                description: cTrId("hbnbota_form_name_desc")
            }

            TextForm {
                htmlId: "domain"
                name: "domain"
                required: true
                minlength: 3
                //: Form field label, domain the contact form will be used on
                //% "Domain"
                label: cTrId("hbnbota_form_domain_label")
                //% "The domain the form will be used on. Includes all subdomains."
                description: cTrId("hbnbota_form_domain_desc")
                placeholder: "example.com"
                pattern: "^[a-zA-Z]+[a-zA-Z0-9\\-_\\.]*\.[a-zA-Z]{2,}$"
            }

            TextAreaForm {
                htmlId: "description"
                name: "description"
                //: Form field label, description for the contact form, used internally
                //% "Description (optional)"
                label: cTrId("hbnbota_form_description_label")
                //% "The description is only used internally."
                description: cTrId("hbnbota_form_description_desc")
            }
        },
        Fieldset {
            htmlId: "addFormFields"
            //: Fieldset legend
            //% "Form fields"
            legend: cTrId("hbnbota_form_fields_fieldset_legend")

            TextForm {
                htmlId: "formFieldSenderName"
                name: "formFieldSenderName"
                //: Form field label
                //% "Sender name field name"
                label: cTrId("hbnbota_form_fields_name_label")
            }

            CheckBoxForm {
                htmlId: "formFieldSenderNameRequired"
                name: "formFieldSenderNameRequired"
                //: Form checkbox label
                //% "Sender name required"
                label: cTrId("hbnbota_form_fields_name_required_label")
            }

            TextForm {
                htmlId: "formFieldSenderEmail"
                name: "formFieldSenderEmail"
                //: Form field label
                //% "Sender email address field name"
                label: cTrId("hbnbota_form_fields_email_label")
            }

            CheckBoxForm {
                htmlId: "formFieldSenderEmailRequired"
                name: "formFieldSenderEmailRequired"
                //: Form checkbox label
                //% "Sender email address required"
                label: cTrId("hbnbota_form_fields_email_required_label")
            }

            TextForm {
                htmlId: "formFieldSenderPhone"
                name: "formFieldSenderPhone"
                //: Form field label
                //% "Sender phone number field name"
                label: cTrId("hbnbota_form_fields_phone_label")
            }

            CheckBoxForm {
                htmlId: "formFieldSenderPhoneRequired"
                name: "formFieldSenderPhoneRequired"
                //: Form checkbox label
                //% "Sender phone number required"
                label: cTrId("hbnbota_form_fields_phone_required_label")
            }

            TextForm {
                htmlId: "formFieldSenderUrl"
                name: "formFieldSenderUrl"
                //: Form field label
                //% "Sender URL field name"
                label: cTrId("hbnbota_form_fields_url_label")
            }

            CheckBoxForm {
                htmlId: "formFieldSenderUrlRequired"
                name: "formFieldSenderUrlRequired"
                //: Form checkbox label
                //% "Sender URL required"
                label: cTrId("hbnbota_form_fields_url_required_label")
            }

            TextForm {
                htmlId: "formFieldSubject"
                name: "formFieldSubject"
                //: Form field label
                //% "Message subject field name"
                label: cTrId("hbnbota_form_fields_subject_label")
            }

            CheckBoxForm {
                htmlId: "formFieldSubjectRequired"
                name: "formFieldSubjectRequired"
                //: Form checkbox label
                //% "Message subject required"
                label: cTrId("hbnbota_form_fields_subject_required_label")
            }

            TextForm {
                htmlId: "formFieldContent"
                name: "formFieldContent"
                //: Form field label
                //% "Message content field name"
                label: cTrId("hbnbota_form_fields_content_label")
            }

            CheckBoxForm {
                htmlId: "formFieldContentRequired"
                name: "formFieldContentRequired"
                //: Form checkbox label
                //% "Message content required"
                label: cTrId("hbnbota_form_fields_content_required_label")
            }
        },
        Fieldset {
            htmlId: "addFormHoneypots"
            //: Fieldset legend
            //% "Honeypots"
            legend: cTrId("hbnbota_form_honeypots_fieldset_legend")

            TextAreaForm {
                htmlId: "honeypots"
                name: "honeypots"
            }
        },
        Fieldset {
            htmlId: "addFormSender"
            //: Fieldset legend
            //% "Sender data"
            legend: cTrId("hbnbota_form_sender_fieldset_legend")

            Select {
                htmlId: "senderType"
                name: "senderType"
                required: true
                //: Form field label
                //% "Mailer"
                label: cTrId("hbnbota_form_sender_sendertype_label")
            }

            TextForm {
                htmlId: "smtpHost"
                name: "smtpHost"
                required: true
                minlength: 3
                placeholder: "smtp.example.net"
                pattern: "^[a-zA-Z]+[a-zA-Z0-9\\-_\\.]*\.[a-zA-Z]{2,}$"
                //: Form field label
                //% "SMTP host"
                label: cTrId("hbnbota_form_sender_smtphost_label")
            }

            NumberForm {
                htmlId: "smtpPort"
                name: "smtpPort"
                required: true
                min: 1
                max: 65535
                //: Form field label
                //% "SMTP port"
                label: cTrId("hbnbota_form_sender_smtpport_label")
                value: 465
            }

            TextForm {
                htmlId: "smtpUser"
                name: "smtpUser"
                required: true
                //: Form field label
                //% "SMTP user"
                label: cTrId("hbnbota_form_sender_smtpuser_label")
            }

            PasswordForm {
                htmlId: "smtpPassword"
                name: "smtpPassword"
                required: true;
                //: Form field label
                //% "SMTP password"
                label: cTrId("hbnbota_form_sender_smtppassword_label")
            }

            Select {
                htmlId: "smtpEncryption"
                name: "smtpEncryption"
                required: true
                //: Form field label
                //% "SMTP encryption method"
                label: cTrId("hbnbota_form_sender_smtpencryption_label")
            }

            Select {
                htmlId: "smtpAuthentication"
                name: "smtpAuthentication"
                required: true
                //: Form field label
                //% "SMTP authentication method"
                label: cTrId("hbnbota_form_sender_smtpauthentication_label")
            }
        }
    ]

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
