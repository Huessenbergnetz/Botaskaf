/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

// const bsMainMenuOffcanvas = new Offcanvas('#sidebarMenu');
const sidebarMenuOffcanvas = document.getElementById('sidebarMenu');
if (sidebarMenuOffcanvas) {
    import('bootstrap/offcanvas.js').then((Offcanvas) => {new Offcanvas.default(sidebarMenuOffcanvas)});
}

// const bsDropdownElementList = document.querySelectorAll('.dropdown-toggle');
// const bsDropdownList = [...bsDropdownElementList].map(dropdownToggleEl => new Dropdown(dropdownToggleEl));

const bsAlertElementList = document.querySelectorAll('.alert');
if (bsAlertElementList.length > 0) {
    import('bootstrap/alert.js').then((Alert) => {
        [...bsAlertElementList].map[el => new Alert.default(el)]
    });
}

const bsTooltipElementList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
if (bsTooltipElementList.length > 0) {
    import('bootstrap/tooltip.js').then((Tooltip) => {
        [...bsTooltipElementList].map(el => new Tooltip.default(el))
    });
}
