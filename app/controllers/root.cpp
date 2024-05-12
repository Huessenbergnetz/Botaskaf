/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "root.h"

#include "logging.h"
#include "objects/error.h"
#include "objects/menuitem.h"
#include "objects/user.h"
#include "settings.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>

using namespace Qt::Literals::StringLiterals;

Root::Root(QObject *parent)
    : Controller(parent)
{
}

void Root::index(Context *c)
{
    c->stash({{u"template"_s, u"index.html"_s},
              //% "Dashboard"
              {u"site_title"_s, c->qtTrId("hbnbota_site_title_dashboard")}});
}

void Root::defaultPage(Context *c)
{
    c->response()->body() = "Page not found!";
    c->response()->setStatus(Response::NotFound);
}

bool Root::Auto(Context *c)
{
    if (c->controllerName() == "Login"_L1) {
        return true;
    }

    const auto user = Authentication::user(c);

    if (Q_UNLIKELY(user.isNull())) {
        c->res()->redirect(c->uriFor(u"/login"_s));
        return false;
    }

    Error e;
    User::toStash(c, e, user);

    c->stash({{u"site_name"_s, Settings::siteName()}});

    return true;
}

void Root::End(Context *c)
{
    if (c->controllerName() == "Login"_L1 || c->controllerName() == "Logout"_L1) {
        return;
    }

    buildUserMenu(c);
    buildMainMenu(c);
}

void Root::buildUserMenu(Context *c)
{
    MenuItemList userMenu;

    //% "Logout"
    userMenu.emplace_back(c, u"userMenuLogout"_s, c->qtTrId("hbnbota_usermenu_logout"), u"index"_s, u"logout"_s);

    c->setStash(u"user_menu"_s, QVariant::fromValue<MenuItemList>(userMenu));
}

void Root::buildMainMenu(Context *c)
{
    MenuItemList mainMenu;

    //% "Dashboard"
    mainMenu.emplace_back(c, u"mainMenuDashboard"_s, c->qtTrId("hbnbota_mainmenu_dashboard"), u"index"_s);

    c->setStash(u"main_menu"_s, QVariant::fromValue<MenuItemList>(mainMenu));
}

#include "moc_root.cpp"
