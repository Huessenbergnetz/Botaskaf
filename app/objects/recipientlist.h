/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_RECIPIENTLIST_H
#define HBNBOTA_RECIPIENTLIST_H

#include "objects/error.h"
#include "objects/form.h"
#include "objects/recipient.h"

#include <QObject>
#include <QSharedData>

namespace Cutelyst {
class Context;
}

class RecipientList
{
    Q_GADGET
    Q_PROPERTY(QList<Recipient> entries READ entries CONSTANT)
    Q_PROPERTY(int size READ size CONSTANT)
    Q_PROPERTY(Error error READ error CONSTANT)
    Q_PROPERTY(bool hasError READ hasError CONSTANT)
    Q_PROPERTY(QMap<QString, QString> labels READ labels CONSTANT)
    Q_PROPERTY(bool isEmpty READ isEmpty CONSTANT)
public:
    RecipientList() noexcept = default;
    RecipientList(const QList<Recipient> &recipients);
    RecipientList(QList<Recipient> &&recipients);
    RecipientList(const Error &error);
    RecipientList(Error &&error);

    RecipientList(const RecipientList &other) noexcept            = default;
    RecipientList(RecipientList &&other) noexcept                 = default;
    RecipientList &operator=(const RecipientList &other) noexcept = default;
    RecipientList &operator=(RecipientList &&other) noexcept      = default;
    ~RecipientList() noexcept                                     = default;

    void swap(RecipientList &other) noexcept { data.swap(other.data); }

    [[nodiscard]] QList<Recipient> entries() const noexcept;

    [[nodiscard]] int size() const noexcept;

    [[nodiscard]] Error error() const noexcept;

    [[nodiscard]] bool hasError() const noexcept;

    [[nodiscard]] QMap<QString, QString> labels() const noexcept;

    [[nodiscard]] bool isEmpty() const noexcept;

    static RecipientList get(Cutelyst::Context *c, const Form &form);

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;

        Data(const QList<Recipient> &_entries);
        Data(QList<Recipient> &&_entries);
        Data(const Error &_error);
        Data(Error &&_error);

        Data(const Data &) noexcept   = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept              = default;

        void loadLabels(Cutelyst::Context *c);

        QList<Recipient> entries;
        Error error;
        QMap<QString, QString> labels;
    };

    QSharedDataPointer<Data> data;
};

Q_DECLARE_SHARED(RecipientList) // NOLINT(modernize-type-traits)
Q_DECLARE_METATYPE(RecipientList)

#endif // HBNBOTA_RECIPIENTLIST_H
