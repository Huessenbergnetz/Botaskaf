/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

import Dropdown from '/js/bootstrap/src/dropdown.js';
import Alert from '/js/bootstrap/src/alert.js';
import Offcanvas from '/js/bootstrap/src/offcanvas.js';

const bsMainMenuOffcanvas = new Offcanvas('#sidebarMenu');

const bsDropdownElementList = document.querySelectorAll('.dropdown-toggle');
const bsDropdownList = [...bsDropdownElementList].map(dropdownToggleEl => new Dropdown(dropdownToggleEl));

const bsAlertElementList = document.querySelectorAll('.alert');
const bsAlertList = [...bsAlertElementList].map(alertEl => new Alert(alertEl));
