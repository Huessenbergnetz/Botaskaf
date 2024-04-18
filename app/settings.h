/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_SETTINGS_H
#define HBNBOTA_SETTINGS_H

#include <QString>

namespace Settings {

bool load(const QVariantMap &core);

/*!
 * \brief The setup token used to initialize the setup.
 *
 * \par Section
 * core
 *
 * \par Key
 * setuptoken
 */
QString setupToken();

/*!
 * \brief The name of the template to use.
 *
 * \par Section
 * core
 *
 * \par Key
 * template
 */
QString tmpl();

/*!
 * \brief Full path to the used template.
 */
QString tmplPath();

/*!
 * \brief Full \a path below the current template’s path.
 */
QString tmplPath(QStringView path);

/*!
 * \brief The main site name.
 */
QString siteName();
} // namespace Settings

#endif // HBNBOTA_SETTINGS
