/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_MENUITEM_H
#define HBNBOTA_MENUITEM_H

#include <Cutelyst/ParamsMultiMap>

#include <QList>
#include <QObject>
#include <QSharedDataPointer>
#include <QUrl>

namespace Cutelyst {
class Context;
}

class MenuItem
{
    Q_GADGET
    Q_PROPERTY(QList<MenuItem> children READ children CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(QUrl image READ image CONSTANT)
    Q_PROPERTY(QUrl url READ url CONSTANT)
    Q_PROPERTY(bool isActive READ isActive CONSTANT)
    Q_PROPERTY(bool hasChildren READ hasChildren CONSTANT)
    Q_PROPERTY(bool isExpanded READ isExpanded CONSTANT)
public:
    MenuItem() noexcept = default;
    MenuItem(Cutelyst::Context *c,
             const QString &name,
             const QString &title,
             const QString &action,
             const QString &ns                           = QString(),
             const QStringList &captures                 = QStringList(),
             const QStringList &args                     = QStringList(),
             const Cutelyst::ParamsMultiMap &queryValues = Cutelyst::ParamsMultiMap());
    MenuItem(Cutelyst::Context *c,
             const QString &name,
             const QString &title,
             const QUrl &image,
             const QString &action,
             const QString &ns                           = QString(),
             const QStringList &captures                 = QStringList(),
             const QStringList &args                     = QStringList(),
             const Cutelyst::ParamsMultiMap &queryValues = Cutelyst::ParamsMultiMap());
    MenuItem(const QString &name, const QString &title, const QUrl &url, bool isActive = false);
    MenuItem(const QString &name, const QString &title, const QString &url, bool isActive = false);

    MenuItem(const MenuItem &other) noexcept            = default;
    MenuItem(MenuItem &&other) noexcept                 = default;
    MenuItem &operator=(const MenuItem &other) noexcept = default;
    MenuItem &operator=(MenuItem &&other) noexcept      = default;
    ~MenuItem() noexcept                                = default;

    void swap(MenuItem &other) noexcept { data.swap(other.data); }

    [[nodiscard]] QList<MenuItem> children() const noexcept;

    [[nodiscard]] QString name() const noexcept;

    [[nodiscard]] QString title() const noexcept;

    [[nodiscard]] QString icon() const noexcept;

    [[nodiscard]] QUrl image() const noexcept;

    [[nodiscard]] QUrl url() const noexcept;

    [[nodiscard]] bool isActive() const noexcept;

    [[nodiscard]] bool hasChildren() const noexcept;

    [[nodiscard]] bool isExpanded() const noexcept;

    void addChildItem(const MenuItem &child);

    MenuItem &addChildItem(Cutelyst::Context *c,
                           const QString &name,
                           const QString &title,
                           const QString &action,
                           const QString &ns                           = QString(),
                           const QStringList &captures                 = QStringList(),
                           const QStringList &args                     = QStringList(),
                           const Cutelyst::ParamsMultiMap &queryValues = Cutelyst::ParamsMultiMap());

    MenuItem &addChildItem(const QString &name, const QString &title, const QUrl &url, bool isActive = false);

    MenuItem &addChildItem(const QString &name, const QString &title, const QString &url, bool isActive = false);

    bool operator==(const MenuItem &other) const noexcept;

    bool operator!=(const MenuItem &other) const noexcept { return !(*this == other); }

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;
        Data(Cutelyst::Context *_c,
             const QString &_name,
             const QString &_title,
             const QString &_action,
             const QString &_ns                           = QString(),
             const QStringList &_captures                 = QStringList(),
             const QStringList &_args                     = QStringList(),
             const Cutelyst::ParamsMultiMap &_queryValues = Cutelyst::ParamsMultiMap());
        Data(Cutelyst::Context *_c,
             const QString &_name,
             const QString &_title,
             const QUrl &_image,
             const QString &_action,
             const QString &_ns,
             const QStringList &_captures,
             const QStringList &_args,
             const Cutelyst::ParamsMultiMap &_queryValues);
        Data(const QString &_name, const QString &_title, const QUrl &_url, bool _isActive = false);
        Data(const QString &_name, const QString &_title, const QString &_url, bool _isActive = false);
        Data(const Data &) noexcept   = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept              = default;

        bool isActive{false};
        bool isExpanded{false};
        QList<MenuItem> children;
        QString name;
        QString title;
        QString icon;
        QUrl image;
        QUrl url;
    };

    QSharedDataPointer<Data> data;
};

Q_DECLARE_SHARED(MenuItem) // NOLINT(modernize-type-traits)

using MenuItemList = QList<MenuItem>;

Q_DECLARE_METATYPE(MenuItem)

#endif // HBNBOTA_MENUITEM_H
