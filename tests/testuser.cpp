/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "objects/user.h"

#include <limits>

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
    void testComparison();
    void testDatastream();
    void testToJson();
    void testTypeStringToEnum();
    void testTypeEnumToString();
    void testSupportedTypes();
    void testToDbId();
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

void UserTest::testComparison()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();

    User u1(
        1, User::Registered, u"user@example.net"_s, u"John Doe"_s, now, now, now, {}, 0, {}, {});
    User u2(2, User::Manager, u"user@example.com"_s, u"Jane Doe"_s, now, now, now, {}, 0, {}, {});
    User u3(
        1, User::Registered, u"user@example.net"_s, u"John Doe"_s, now, now, now, {}, 0, {}, {});
    User u4;
    User u5(0, User::Invalid, {}, {}, {}, {}, {}, {}, 0, {}, {});
    User u6 = u1;

    QVERIFY(u1 != u2);
    QVERIFY(u1 == u3);
    QVERIFY(u1 != u4);
    QVERIFY(u1 != u5);
    QVERIFY(u1 == u6);

    QVERIFY(u4 == u5);
}

void UserTest::testDatastream()
{
    const QDateTime now      = QDateTime::currentDateTimeUtc();
    const QDateTime created  = now.addDays(-1);
    const QDateTime updated  = now.addSecs(-5000);
    const QDateTime lastSeen = now.addSecs(-60);
    const QVariantMap settings =
        QVariantMap({{u"timezone"_s, u"Europe/Berlin"_s}, {u"locale"_s, u"de_DE"_s}});

    // test valid into null
    {
        User u1(1,
                User::Manager,
                u"user@example.net"_s,
                u"John Doe"_s,
                created,
                updated,
                lastSeen,
                now,
                2,
                u"Jane Doe"_s,
                settings);

        QByteArray outBa;
        QDataStream out(&outBa, QIODeviceBase::WriteOnly);
        out << u1;

        const QByteArray inBa = outBa;
        QDataStream in(inBa);
        User u2;
        in >> u2;

        QCOMPARE(u1, u2);
    }

    // test valid into valid
    {
        User u1(1,
                User::Manager,
                u"user@example.net"_s,
                u"John Doe"_s,
                created,
                updated,
                lastSeen,
                now,
                2,
                u"Jane Doe"_s,
                settings);
        User u2(2,
                User::Administrator,
                u"user@example.com"_s,
                u"Jane Doe"_s,
                created.addSecs(1),
                updated.addMSecs(2),
                lastSeen.addSecs(1),
                now.addSecs(3),
                1,
                u"John Doe"_s,
                {});

        QByteArray outBa;
        QDataStream out(&outBa, QIODeviceBase::WriteOnly);
        out << u1;

        const QByteArray inBa = outBa;
        QDataStream in(inBa);
        in >> u2;

        QCOMPARE(u1, u2);
    }

    // test null into valid
    {
        User u1;
        User u2(2,
                User::Administrator,
                u"user@example.com"_s,
                u"Jane Doe"_s,
                created.addSecs(1),
                updated.addMSecs(2),
                lastSeen.addSecs(1),
                now.addSecs(3),
                1,
                u"John Doe"_s,
                {});

        QByteArray outBa;
        QDataStream out(&outBa, QIODeviceBase::WriteOnly);
        out << u1;

        const QByteArray inBa = outBa;
        QDataStream in(inBa);
        in >> u2;

        QVERIFY(u2.isNull());
    }

    // test null into null
    {
        User u1, u2;

        QByteArray outBa;
        QDataStream out(&outBa, QIODeviceBase::WriteOnly);
        out << u1;

        const QByteArray inBa = outBa;
        QDataStream in(inBa);
        in >> u2;

        QVERIFY(u2.isNull());
    }
}

void UserTest::testToJson()
{
    const QDateTime now      = QDateTime::currentDateTimeUtc();
    const QDateTime created  = now.addDays(-1);
    const QDateTime updated  = now.addSecs(-5000);
    const QDateTime lastSeen = now.addSecs(-60);
    const QVariantMap settings =
        QVariantMap({{u"timezone"_s, u"Europe/Berlin"_s}, {u"locale"_s, u"de_DE"_s}});

    User u1(1,
            User::Manager,
            u"user@example.net"_s,
            u"John Doe"_s,
            created,
            updated,
            lastSeen,
            now,
            2,
            u"Jane Doe"_s,
            settings);

    QJsonObject json = u1.toJson();
    QCOMPARE(json.value("id"_L1), QJsonValue(1));
    QCOMPARE(json.value("type"_L1), QJsonValue(static_cast<int>(User::Manager)));
    QCOMPARE(json.value("email"_L1), QJsonValue(u"user@example.net"_s));
    QCOMPARE(json.value("displayName"_L1), QJsonValue(u"John Doe"_s));
    QCOMPARE(json.value("created"_L1), QJsonValue(created.toString(Qt::ISODateWithMs)));
    QCOMPARE(json.value("updated"_L1), QJsonValue(updated.toString(Qt::ISODateWithMs)));
    QCOMPARE(json.value("lastSeen"_L1), QJsonValue(lastSeen.toString(Qt::ISODateWithMs)));
    QCOMPARE(json.value("lockedAt"_L1), QJsonValue(now.toString(Qt::ISODateWithMs)));
    QCOMPARE(json.value("lockedById"_L1), QJsonValue(2));
    QCOMPARE(json.value("lockedByName"_L1), QJsonValue(u"Jane Doe"_s));
    QCOMPARE(json.value("settings"_L1), QJsonValue(QJsonObject::fromVariantMap(settings)));

    User u2(2,
            User::Administrator,
            u"user@example.com"_s,
            u"Jane Doe"_s,
            created,
            updated,
            {},
            {},
            0,
            {},
            {});
    json = u2.toJson();
    QCOMPARE(json.value("lastSeen"_L1), QJsonValue());
    QCOMPARE(json.value("lockedAt"_L1), QJsonValue());
    QCOMPARE(json.value("lockedByName"_L1), QJsonValue());

    User u3;
    json = u3.toJson();
    QVERIFY(json.isEmpty());
}

void UserTest::testTypeStringToEnum()
{
    QCOMPARE(User::typeStringToEnum(u"invalid"_s), User::Invalid);
    QCOMPARE(User::typeStringToEnum(u"KatZenSCHEIsse"_s), User::Invalid);
    QCOMPARE(User::typeStringToEnum(u"DiSabled"_s), User::Disabled);
    QCOMPARE(User::typeStringToEnum(u"reGIstered"_s), User::Registered);
    QCOMPARE(User::typeStringToEnum(u"Manager"_s), User::Manager);
    QCOMPARE(User::typeStringToEnum(u"AdMiNiStRaToR"_s), User::Administrator);
    QCOMPARE(User::typeStringToEnum(u"SuperUser"_s), User::SuperUser);
}

void UserTest::testTypeEnumToString()
{
    QCOMPARE(User::typeEnumToString(User::Manager), u"Manager"_s);
}

void UserTest::testSupportedTypes()
{
    const QStringList lst = User::supportedTypes();
    QVERIFY(!lst.empty());
    QVERIFY(lst.contains(u"Administrator"));
    QVERIFY(!lst.contains(u"Invalid"));
}

void UserTest::testToDbId()
{
    bool ok         = false;
    User::dbid_t id = 0;

    id = User::toDbId(123, &ok);
    QVERIFY(ok);
    QCOMPARE(id, 123);

    id = User::toDbId(-1, &ok);
    QVERIFY(!ok);
    QCOMPARE(id, 0);

    id = User::toDbId(static_cast<qulonglong>(std::numeric_limits<User::dbid_t>::max()) + 4, &ok);
    QVERIFY(!ok);
    QCOMPARE(id, 0);

    id = User::toDbId(QVariant::fromValue(123), &ok);
    QVERIFY(ok);
    QCOMPARE(id, 123);

    id = User::toDbId(QVariant::fromValue(-1), &ok);
    QVERIFY(!ok);
    QCOMPARE(id, 0);

    id = User::toDbId(
        QVariant::fromValue(static_cast<qulonglong>(std::numeric_limits<User::dbid_t>::max()) + 4),
        &ok);
    QVERIFY(!ok);
    QCOMPARE(id, 0);

    id = User::toDbId(u"123"_s, &ok);
    QVERIFY(ok);
    QCOMPARE(id, 123);

    id = User::toDbId(u"-1"_s, &ok);
    QVERIFY(!ok);
    QCOMPARE(id, 0);

    id = User::toDbId(u"4294967299"_s, &ok);
    QVERIFY(!ok);
    QCOMPARE(id, 0);
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)

QTEST_MAIN(UserTest)

#include "testuser.moc"
