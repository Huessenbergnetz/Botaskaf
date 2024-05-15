/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

import Dropdown from 'bootstrap/dropdown.js';
import Alert from 'bootstrap/alert.js';
import Offcanvas from 'bootstrap/offcanvas.js';

const bsMainMenuOffcanvas = new Offcanvas('#sidebarMenu');

const bsDropdownElementList = document.querySelectorAll('.dropdown-toggle');
const bsDropdownList = [...bsDropdownElementList].map(dropdownToggleEl => new Dropdown(dropdownToggleEl));

const bsAlertElementList = document.querySelectorAll('.alert');
const bsAlertList = [...bsAlertElementList].map(alertEl => new Alert(alertEl));

const bsTooltipElementList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
if (bsTooltipElementList.length > 0) {
    import('bootstrap/tooltip.js').then((Tooltip) => {
        [...bsTooltipElementList].map(el => new Tooltip.default(el))
    });
}
