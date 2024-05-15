// SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
// SPDX-License-Identifier: AGPL-3.0-or-later
import de.huessenbergnetz.cutelystforms 1.0

Form {
    //: Form label, migth be used as form header
    //% "Welcome to the Botaskaf setup"
    label: cTrId("hbnbota_form_setup_index_label")
    //: Form description
    //% "To begin the setup process, please enter the keyword you set in your "
    //% "configuration file in the “core” section under the key “setuptoken”."
    description: cTrId("hbnbota_form_setup_index_desc")
    method: Form.Post

    TextForm {
        htmlId: "setuptoken"
        name: "setuptoken"
        required: true
        //: Form field label
        //% "Setup token"
        label: cTrId("hbnbota_form_setup_index_setuptoken_label")
    }

    buttons: [
        FormButton {
            htmlId: "submitBtn"
            //: Form button text
            //% "Next"
            text: cTrId("hbnbota_general_next")
        }
    ]
}
