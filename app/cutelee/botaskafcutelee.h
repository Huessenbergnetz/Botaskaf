/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_BOTASKAFCUTELEE_H
#define HBNBOTA_BOTASKAFCUTELEE_H

#include <cutelee/taglibraryinterface.h>

#include <QObject>

using namespace Cutelee;

class BotaskafCutelee final
    : public QObject
    , public TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(Cutelee::TagLibraryInterface)
public:
    explicit BotaskafCutelee(QObject *parent = nullptr);
    ~BotaskafCutelee() override = default;

    QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = QString()) override;

    QHash<QString, Filter *> filters(const QString &name = QString()) override;

private:
    Q_DISABLE_COPY(BotaskafCutelee)
};

#endif // HBNBOTA_BOTASKAFCUTELEE_H
