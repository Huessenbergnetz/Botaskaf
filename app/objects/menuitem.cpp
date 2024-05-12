/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "menuitem.h"

#include "settings.h"

#include <Cutelyst/Context>

MenuItem::Data::Data(Cutelyst::Context *_c,
                     const QString &_name,
                     const QString &_title,
                     const QString &_action,
                     const QString &_ns,
                     const QStringList &_captures,
                     const QStringList &_args,
                     const Cutelyst::ParamsMultiMap &_queryValues)
    : QSharedData()
    , name{_name}
    , title{_title}
{
    auto _act = _c->getAction(_action, _ns);
    Q_ASSERT_X(_act, "create new MenuItem", "can not find action");
    url      = _c->uriFor(_act, _captures, _args, _queryValues);
    isActive = _act == _c->action();
    icon     = Settings::tmplIcon(_name);
}

MenuItem::Data::Data(Cutelyst::Context *_c,
                     const QString &_name,
                     const QString &_title,
                     const QUrl &_image,
                     const QString &_action,
                     const QString &_ns,
                     const QStringList &_captures,
                     const QStringList &_args,
                     const Cutelyst::ParamsMultiMap &_queryValues)
    : QSharedData()
    , name{_name}
    , title{_title}
    , image{_image}
{
    auto _act = _c->getAction(_action, _ns);
    Q_ASSERT_X(_act, "create new MenuItem", "can not find action");
    url      = _c->uriFor(_act, _captures, _args, _queryValues);
    isActive = _act == _c->action();
}

MenuItem::Data::Data(const QString &_name, const QString &_title, const QUrl &_url, bool _isActive)
    : QSharedData()
    , isActive{_isActive}
    , name{_name}
    , title{_title}
    , url{_url}
{
    Q_ASSERT_X(url.isValid(), "create new MenuItem", "invalid URL");
    icon = Settings::tmplIcon(_name);
}

MenuItem::Data::Data(const QString &_name, const QString &_title, const QString &_url, bool _isActive)
    : Data{_name, _title, QUrl{_url}, _isActive}
{
}

MenuItem::MenuItem(Cutelyst::Context *c,
                   const QString &name,
                   const QString &title,
                   const QString &action,
                   const QString &ns,
                   const QStringList &captures,
                   const QStringList &args,
                   const Cutelyst::ParamsMultiMap &queryValues)
    : data{new Data{c, name, title, action, ns, captures, args, queryValues}}
{
}

MenuItem::MenuItem(Cutelyst::Context *c,
                   const QString &name,
                   const QString &title,
                   const QUrl &image,
                   const QString &action,
                   const QString &ns,
                   const QStringList &captures,
                   const QStringList &args,
                   const Cutelyst::ParamsMultiMap &queryValues)
    : data{new Data{c, name, title, image, action, ns, captures, args, queryValues}}
{
}

MenuItem::MenuItem(const QString &name, const QString &title, const QUrl &url, bool isActive)
    : data{new Data{name, title, url, isActive}}
{
}

MenuItem::MenuItem(const QString &name, const QString &title, const QString &url, bool isActive)
    : data{new Data{name, title, url, isActive}}
{
}

QList<MenuItem> MenuItem::children() const noexcept
{
    return data ? data->children : QList<MenuItem>();
}

QString MenuItem::name() const noexcept
{
    return data ? data->name : QString();
}

QString MenuItem::title() const noexcept
{
    return data ? data->title : QString();
}

QString MenuItem::icon() const noexcept
{
    return data ? data->icon : QString();
}

QUrl MenuItem::image() const noexcept
{
    return data ? data->image : QUrl();
}

QUrl MenuItem::url() const noexcept
{
    return data ? data->url : QUrl();
}

bool MenuItem::isActive() const noexcept
{
    return data ? data->isActive : false;
}

bool MenuItem::hasChildren() const noexcept
{
    return data ? !data->children.empty() : false;
}

bool MenuItem::isExpanded() const noexcept
{
    return data ? data->isExpanded : false;
}

void MenuItem::addChildItem(const MenuItem &child)
{
    Q_ASSERT_X(data, "MenuItem::addChildItem", "can not add a child to a NULL MenuItem");
    data->children.push_back(child);
    data->isExpanded = child.isActive();
}

MenuItem &MenuItem::addChildItem(Cutelyst::Context *c,
                                 const QString &name,
                                 const QString &title,
                                 const QString &action,
                                 const QString &ns,
                                 const QStringList &captures,
                                 const QStringList &args,
                                 const Cutelyst::ParamsMultiMap &queryValues)
{
    Q_ASSERT_X(data, "MenuItem::addChildItem", "can not add a child to a NULL MenuItem");
    auto &ref        = data->children.emplace_back(c, name, title, action, ns, captures, args, queryValues);
    data->isExpanded = ref.isActive();
    return ref;
}

MenuItem &MenuItem::addChildItem(const QString &name, const QString &title, const QUrl &url, bool isActive)
{
    Q_ASSERT_X(data, "MenuItem::addChildItem", "can not add a child to a NULL MenuItem");
    auto &ref        = data->children.emplace_back(name, title, url, isActive);
    data->isExpanded = ref.isActive();
    return ref;
}

MenuItem &MenuItem::addChildItem(const QString &name, const QString &title, const QString &url, bool isActive)
{
    Q_ASSERT_X(data, "MenuItem::addChildItem", "can not add a child to a NULL MenuItem");
    auto &ref        = data->children.emplace_back(name, title, url, isActive);
    data->isExpanded = ref.isActive();
    return ref;
}

bool MenuItem::operator==(const MenuItem &other) const noexcept
{
    if (data == other.data)
        return true;

    if (name() != other.name())
        return false;

    if (title() != other.title())
        return false;

    if (icon() != other.icon())
        return false;

    if (image() != other.image())
        return false;

    if (url() != other.url())
        return false;

    if (isActive() != other.isActive())
        return false;

    if (hasChildren() != other.hasChildren())
        return false;

    if (isExpanded() != other.isExpanded())
        return false;

    if (children() != other.children())
        return false;

    return true;
}

#include "moc_menuitem.cpp"
