/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "objects/error.h"

#include <Cutelyst/Context>

#include <QSqlError>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class ErrorObjectTest final : public QObject
{
    Q_OBJECT
public:
    explicit ErrorObjectTest(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private slots:
    void initTestCase();
    void testDefaultConstructor();
    void testNormaConstructor();
    void testSqlErrorConstructor();
    void testCopy();
    void testMove();
    void testSwap();
    void testComparison();

private:
    std::unique_ptr<Cutelyst::Context> m_c;
};

void ErrorObjectTest::initTestCase()
{
    m_c = std::make_unique<Cutelyst::Context>();
}

void ErrorObjectTest::testDefaultConstructor()
{
    Error e;
    QCOMPARE(e.status(), Cutelyst::Response::OK);
    QVERIFY(e.text().isNull());
    QVERIFY(e.sqlErrorText().isEmpty());
    QVERIFY(e.code().isNull());
}

void ErrorObjectTest::testNormaConstructor()
{
    Error e1 = Error::create(Cutelyst::Response::MethodNotAllowed, u"Only POST is allowed"_s);
    QCOMPARE(e1.status(), Cutelyst::Response::MethodNotAllowed);
    QCOMPARE(e1.text(), u"Only POST is allowed"_s);
    QVERIFY(e1.code().isNull());

    Error e2{Cutelyst::Response::MethodNotAllowed, u"Only POST is allowed"_s, u"My_Error_CODE"_s};
    QCOMPARE(e2.code(), u"MY_ERROR_CODE"_s);
}

void ErrorObjectTest::testSqlErrorConstructor()
{
    const QSqlError sqlError1{u"Little Driver Text"_s, u"Little Database Text"_s, QSqlError::StatementError};
    Error e1(sqlError1, u"Something failed in the database"_s);
    QCOMPARE(e1.text(), u"Something failed in the database"_s);
    QCOMPARE(e1.status(), Cutelyst::Response::InternalServerError);

    const QSqlError sqlError2{
        u"Little Driver Text"_s, u"Little Database Text"_s, QSqlError::StatementError, u"db_error_code"_s};
    Error e2(sqlError2, u"Something failed in the database"_s);
    QCOMPARE(e2.code(), u"DB_ERROR_CODE"_s);
}

void ErrorObjectTest::testCopy()
{
    // test copy constructor
    {
        Error e1(Cutelyst::Response::NotFound, u"Page not found"_s);
        Error e2(e1);

        QCOMPARE(e1.status(), e2.status());
        QCOMPARE(e1.text(), e2.text());
    }

    // test copy assignment
    {
        Error e1(Cutelyst::Response::NotFound, u"Page not found"_s);
        Error e2;
        e2 = e1;

        QCOMPARE(e1.status(), e2.status());
        QCOMPARE(e1.text(), e2.text());
    }
}

void ErrorObjectTest::testMove()
{
    // test move constructor
    {
        Error e1(Cutelyst::Response::NotFound, u"Page not found"_s);
        Error e2(std::move(e1));

        QCOMPARE(e2.status(), Cutelyst::Response::NotFound);
        QCOMPARE(e2.text(), u"Page not found"_s);
    }

    // test move assignment
    {
        Error e1(Cutelyst::Response::NotFound, u"Page not found"_s);
        Error e2(Cutelyst::Response::InternalServerError, u"Invalid config value"_s);
        e2 = std::move(e1);

        QCOMPARE(e2.status(), Cutelyst::Response::NotFound);
        QCOMPARE(e2.text(), u"Page not found"_s);
    }
}

void ErrorObjectTest::testSwap()
{
    Error e1(Cutelyst::Response::NotFound, u"Page not found"_s);
    Error e2(Cutelyst::Response::InternalServerError, u"Invalid config value"_s);
    Error e3;

    swap(e1, e3);
    QCOMPARE(e1.status(), Cutelyst::Response::OK);
    QCOMPARE(e3.status(), Cutelyst::Response::NotFound);
    e3.swap(e2);
    QCOMPARE(e3.status(), Cutelyst::Response::InternalServerError);
    QCOMPARE(e2.status(), Cutelyst::Response::NotFound);
}

void ErrorObjectTest::testComparison()
{
    Error e1(Cutelyst::Response::NotFound, u"Page not found"_s);
    Error e2(Cutelyst::Response::InternalServerError, u"Invalid config value"_s);
    Error e3(Cutelyst::Response::NotFound, u"Page not found"_s);
    Error e4;
    Error e5(Cutelyst::Response::OK, QString());
    Error e6 = e1;

    QVERIFY(e1 != e2);
    QVERIFY(e1 == e3);
    QVERIFY(e1 != e4);
    QVERIFY(e1 != e5);
    QVERIFY(e1 == e6);

    QVERIFY(e4 == e5);
}

QTEST_MAIN(ErrorObjectTest)

#include "testerrorobject.moc"
