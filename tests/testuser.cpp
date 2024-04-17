/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "objects/user.h"

#include <QDataStream>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class UserTest final : public QObject // NOLINT(cppcoreguidelines-special-member-functions)
{
    Q_OBJECT
public:
    explicit UserTest(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~UserTest() override = default;

private slots:
    void testDefaultConstructor();
    void testConstructorWithArgs();
    void testCopy();
    void testMove();
    void testSwap();
    void testClear();
};

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
void UserTest::testDefaultConstructor()
{
    User u;
    QVERIFY(u.isNull());
    QVERIFY(!u.isValid());
    QCOMPARE(u.id(), 0);
    QCOMPARE(u.type(), User::Invalid);
    QVERIFY(u.email().isEmpty());
    QVERIFY(u.displayName().isEmpty());
    QVERIFY(u.created().isNull());
    QVERIFY(u.updated().isNull());
    QVERIFY(u.lastSeen().isNull());
    QVERIFY(u.lockedAt().isNull());
    QCOMPARE(u.lockedById(), 0);
    QVERIFY(u.lockedByName().isEmpty());
    QVERIFY(u.settings().empty());
    QVERIFY(u.timezone().isEmpty());
    QVERIFY(u.locale().isEmpty());
}

void UserTest::testConstructorWithArgs()
{
    const QDateTime now      = QDateTime::currentDateTimeUtc();
    const QDateTime created  = now.addDays(-1);
    const QDateTime updated  = now.addSecs(-5000);
    const QDateTime lastSeen = now.addSecs(-60);
    const QVariantMap settings =
        QVariantMap({{u"timezone"_s, u"Europe/Berlin"_s}, {u"locale"_s, u"de_DE"_s}});

    User u{1,
           User::Registered,
           u"user@example.net"_s,
           u"John Doe"_s,
           created,
           updated,
           lastSeen,
           now,
           2,
           u"Other User"_s,
           settings};

    QCOMPARE(u.id(), 1);
    QCOMPARE(u.type(), User::Registered);
    QCOMPARE(u.email(), u"user@example.net"_s);
    QCOMPARE(u.displayName(), u"John Doe"_s);
    QCOMPARE(u.created(), created);
    QCOMPARE(u.updated(), updated);
    QCOMPARE(u.lastSeen(), lastSeen);
    QCOMPARE(u.lockedAt(), now);
    QCOMPARE(u.lockedById(), 2);
    QCOMPARE(u.lockedByName(), u"Other User"_s);
    QCOMPARE(u.settings(), settings);
    QCOMPARE(u.timezone(), u"Europe/Berlin"_s);
    QCOMPARE(u.locale(), u"de_DE"_s);
}

void UserTest::testCopy()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();

    // test copy constructor
    {
        User u1(1,
                User::Registered,
                u"user@example.net"_s,
                u"John Doe"_s,
                now,
                now,
                now,
                {},
                0,
                {},
                {});
        User u2(u1);

        QCOMPARE(u1.id(), u2.id());
        QCOMPARE(u1.type(), u2.type());
        QCOMPARE(u1.email(), u2.email());
        QCOMPARE(u1.displayName(), u2.displayName());
    }

    // test copy assignment
    {
        User u1(1,
                User::Registered,
                u"user@example.net"_s,
                u"John Doe"_s,
                now,
                now,
                now,
                {},
                0,
                {},
                {});
        User u2;
        u2 = u1;

        QCOMPARE(u1.id(), u2.id());
        QCOMPARE(u1.type(), u2.type());
        QCOMPARE(u1.email(), u2.email());
        QCOMPARE(u1.displayName(), u2.displayName());
    }
}

void UserTest::testMove()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();

    // test move constructor
    {
        User u1(1,
                User::Registered,
                u"user@example.net"_s,
                u"John Doe"_s,
                now,
                now,
                now,
                {},
                0,
                {},
                {});
        User u2(std::move(u1));

        QCOMPARE(u2.id(), 1);
        QCOMPARE(u2.type(), User::Registered);
        QCOMPARE(u2.email(), u"user@example.net"_s);
        QCOMPARE(u2.displayName(), u"John Doe"_s);
    }

    // test move assignment
    {
        User u1(1,
                User::Registered,
                u"user@example.net"_s,
                u"John Doe"_s,
                now,
                now,
                now,
                {},
                0,
                {},
                {});
        User u2(
            2, User::Manager, u"user@example.com"_s, u"Jane Doe"_s, now, now, now, {}, 0, {}, {});
        u2 = std::move(u1);

        QCOMPARE(u2.id(), 1);
        QCOMPARE(u2.type(), User::Registered);
        QCOMPARE(u2.email(), u"user@example.net"_s);
        QCOMPARE(u2.displayName(), u"John Doe"_s);
    }
}

void UserTest::testSwap()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();

    User u1(
        1, User::Registered, u"user@example.net"_s, u"John Doe"_s, now, now, now, {}, 0, {}, {});
    User u2(2, User::Manager, u"user@example.com"_s, u"Jane Doe"_s, now, now, now, {}, 0, {}, {});
    User u3;

    swap(u1, u3);
    QVERIFY(u1.isNull());
    QCOMPARE(u3.id(), 1);
    u3.swap(u2);
    QCOMPARE(u3.id(), 2);
    QCOMPARE(u2.id(), 1);
}

void UserTest::testClear()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    User u1(
        1, User::Registered, u"user@example.net"_s, u"John Doe"_s, now, now, now, {}, 0, {}, {});
    u1.clear();
    QVERIFY(u1.isNull());
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)

QTEST_MAIN(UserTest)

#include "testuser.moc"
