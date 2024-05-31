/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "objects/form.h"

#include <QTest>
#include <QUuid>

using namespace Qt::Literals::StringLiterals;

class FormTest final : public QObject
{
    Q_OBJECT
public:
    explicit FormTest(QObject *parent = nullptr)
        : QObject{parent}
    {
    }
    ~FormTest() override = default;

private slots:
    void testEncryption();
};

void FormTest::testEncryption()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();

    Form f{1,
           u"Testform"_s,
           u"www.example.com"_s,
           {},
           QUuid::createUuid().toString(QUuid::Id128),
           QUuid::createUuid().toString(QUuid::Id128).toUpper(),
           {},
           now,
           {},
           {},
           {},
           {},
           0};

    const QByteArray token = f.encrypt(now);
    QVERIFY(!token.isEmpty());
}

QTEST_MAIN(FormTest)

#include "testform.moc"
