/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "dateformattag.h"

#include "logging.h"
#include "qtimezonevariant_p.h"
#include "settings.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Session/Session>
#include <chrono>
#include <cutelee/exception.h>
#include <cutelee/parser.h>

using namespace Qt::Literals::StringLiterals;

QString getFormat(const FilterExpression &f, Context *cc)
{
    if (f.isValid()) {
        const QVariant var = f.resolve(cc);
        if (var.userType() == qMetaTypeId<SafeString>()) {
            return var.value<SafeString>().get(); // NOLINT(cppcoreguidelines-slicing)
        } else {
            return var.toString();
        }
    }

    return u"short"_s;
}

QString getRelative(Cutelyst::Context *c, qint64 secsTo)
{
    if (secsTo == 0) {
        //: relative time
        //% "now"
        return c->qtTrId("hbnbota_cutelee_dateformattag_rel_now");
    }

    const std::chrono::seconds diff{std::abs(secsTo)};
    if (secsTo < 0) {
        if (diff >= std::chrono::years{1}) {
            //: relative time
            //% "%n year(s) ago"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_yearsago",
                             std::chrono::round<std::chrono::years>(diff).count());
        } else if (diff >= std::chrono::months{1}) {
            //: relative time
            //% "%n month(s) ago"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_monthsago",
                             std::chrono::round<std::chrono::months>(diff).count());
        } else if (diff >= std::chrono::weeks{1}) {
            //: relative time
            //% "%n week(s) ago"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_weeksago",
                             std::chrono::round<std::chrono::weeks>(diff).count());
        } else if (diff >= std::chrono::days{1}) {
            //: relative time
            //% "%n day(s) ago"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_daysago",
                             std::chrono::round<std::chrono::days>(diff).count());
        } else if (diff >= std::chrono::hours{1}) {
            //: relative time
            //% "%n hour(s) ago"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_hoursago",
                             std::chrono::round<std::chrono::hours>(diff).count());
        } else if (diff >= std::chrono::minutes{1}) {
            //: relative time
            //% "%n minute(s) ago"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_minutesago",
                             std::chrono::round<std::chrono::minutes>(diff).count());
        } else {
            //: relative time
            //% "%n second(s) ago"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_secondsago", diff.count());
        }
    } else {
        if (diff >= std::chrono::years{1}) {
            //: relative time
            //% "in %n year(s)"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_inyears",
                             std::chrono::round<std::chrono::years>(diff).count());
        } else if (diff >= std::chrono::months{1}) {
            //: relative time
            //% "in %n month(s)"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_inmonths",
                             std::chrono::round<std::chrono::months>(diff).count());
        } else if (diff >= std::chrono::weeks{1}) {
            //: relative time
            //% "in %n week(s)"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_inweeks",
                             std::chrono::round<std::chrono::weeks>(diff).count());
        } else if (diff >= std::chrono::days{1}) {
            //: relative time
            //% "in %n day(s)"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_indays",
                             std::chrono::round<std::chrono::days>(diff).count());
        } else if (diff >= std::chrono::hours{1}) {
            //: relative time
            //% "in %n hour(s)"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_inhours",
                             std::chrono::round<std::chrono::hours>(diff).count());
        } else if (diff >= std::chrono::minutes{1}) {
            //: relative time
            //% "in %n minute(s)"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_inminutes",
                             std::chrono::round<std::chrono::minutes>(diff).count());
        } else {
            //: relative time
            //% "in %n second(s)"
            return c->qtTrId("hbnbota_cutelee_dateformattag_rel_inseconds", diff.count());
        }
    }
}

QString getRelative(Cutelyst::Context *c, const QDateTime &dt)
{
    if (dt.isNull() || !dt.isValid()) {
        //: relative time
        //% "never"
        return c->qtTrId("hbnbota_cutelee_dateformattag_rel_never");
    }

    const auto secsTo = QDateTime::currentDateTimeUtc().secsTo(dt);

    return getRelative(c, secsTo);
}

QString getRelative(Cutelyst::Context *c, const QDate &date)
{
    if (date.isNull() || !date.isValid()) {
        return c->qtTrId("hbnbota_cutelee_dateformattag_rel_never");
    }

    const auto secsTo = QDate::currentDate().daysTo(date) * 86400;

    return getRelative(c, secsTo);
}

QString getRelative(Cutelyst::Context *c, const QTime &time)
{
    if (time.isNull() || !time.isValid()) {
        return c->qtTrId("hbnbota_cutelee_dateformattag_rel_never");
    }

    const auto secsTo = QTime::currentTime().secsTo(time);

    return getRelative(c, secsTo);
}

QString formatDateTime(Cutelyst::Context *c, const QVariant &dtVar, const QString &format)
{
    QDateTime dt = dtVar.toDateTime();

    if (format == "relative"_L1) {
        return getRelative(c, dt);
    }

    if (dt.isNull() || !dt.isValid()) {
        return {};
    }

    auto userTz = Cutelyst::Session::value(c, u"tz"_s).value<QTimeZone>();
    if (!userTz.isValid()) {
        const QTimeZone defTz = Settings::defTimeZone();
        if (defTz.isValid()) {
            userTz = defTz;
            qCWarning(HBNBOTA_CUTELEE) << "Can not find valid user time zone in session, using default time zone:"
                                       << userTz.id();
        } else {
            userTz = QTimeZone::utc();
        }
    }

    if (userTz != QTimeZone::utc()) {
        dt = dt.toTimeZone(userTz);
    }

    if (format == "short"_L1) {
        //: short format string for a QDateTime value, should fit to the output of JavaScript’s Intl.DateTimeFormat
        //: with dateStyle: short and timeStyle: short
        //% "M/d/yy, h:mm AP"
        return c->locale().toString(dt, c->qtTrId("hbnbota_cutelee_dateformattag_datetime_short"));
    } else if (format == "long"_L1) {
        //: long format string for a QDateTime value, should fit to the output of JavaScript’s Intl.DateTimeFormat
        //: with dateStyle: long and timeStyle: medium
        //% "MMMM d, yyyy 'at' h:mm:ss AP"
        return c->locale().toString(dt, c->qtTrId("hbnbota_cutelee_dateformattag_datetime_long"));
    } else {
        return c->locale().toString(dt, format);
    }
}

QString formatDate(Cutelyst::Context *c, const QVariant &dateVar, const QString &format)
{
    const QDate d = dateVar.toDate();

    if (format == "relative"_L1) {
        return getRelative(c, d);
    }

    if (d.isNull() || !d.isValid()) {
        return {};
    }

    if (format == "short"_L1) {
        //: short format string for a QDate value, should fit to the output of JavaScript’s Intl.DateTimeFormat
        //: with dateStyle: short
        //% "M/d/yy"
        return c->locale().toString(d, c->qtTrId("hbnbota_cutelee_dateformattag_date_short"));
    } else if (format == "long"_L1) {
        //: short format string for a QDate value, should fit to the output of JavaScript’s Intl.DateTimeFormat
        //: with dateStyle: long
        //% "MMMM d, yyyy"
        return c->locale().toString(d, c->qtTrId("hbnbota_cutelee_dateformattag_date_long"));
    } else {
        return c->locale().toString(d, format);
    }
}

QString formatTime(Cutelyst::Context *c, const QVariant &timeVar, const QString &format)
{
    const QTime t = timeVar.toTime();

    if (format == "relative"_L1) {
        return getRelative(c, t);
    }

    if (t.isNull() || !t.isValid()) {
        return {};
    }

    if (format == "short"_L1) {
        //: short format string for a QTime value, should fit to the output of JavaScript’s Intl.DateTimeFormat
        //: with timeStyle: short
        //% "h:mm AP"
        return c->locale().toString(t, c->qtTrId("hbnbota_cutelee_dateformattag_time_short"));
    } else if (format == "long"_L1) {
        //: short format string for a QTime value, should fit to the output of JavaScript’s Intl.DateTimeFormat
        //: with timeStyle: medium
        //% "h:mm:ss AP"
        return c->locale().toString(t, c->qtTrId("hbnbota_cutelee_dateformattag_time_long"));
    } else {
        return c->locale().toString(t, format);
    }
}

Cutelyst::Context *findCutelystContext(QString &cutelystContext, Context *cuteleeContext)
{
    auto c = cuteleeContext->lookup(cutelystContext).value<Cutelyst::Context *>();
    if (!c) {
        const QVariantHash hash = cuteleeContext->stackHash(0);
        auto it                 = hash.constBegin();
        while (it != hash.constEnd()) {
            if (it.value().userType() == qMetaTypeId<Cutelyst::Context *>()) {
                c = it.value().value<Cutelyst::Context *>();
                if (c) {
                    cutelystContext = it.key();
                    break;
                }
            }
            ++it;
        }
    }
    return c;
}

DateFormatTag::DateFormatTag(QObject *parent)
    : AbstractNodeFactory(parent)
{
}

Node *DateFormatTag::getNode(const QString &tagContent, Parser *p) const
{
    QStringList parts = smartSplit(tagContent);
    parts.removeFirst();
    if (parts.empty()) {
        throw Exception(TagSyntaxError, u"hbnbota_dateformat requires at least the datetime, date or time"_s);
    }

    FilterExpression dateTime{parts.at(0), p};

    FilterExpression format;
    if (parts.size() > 1) {
        format = FilterExpression{parts.at(1), p};
    }

    return new DateFormat{dateTime, format, p}; // NOLINT(cppcoreguidelines-owning-memory)
}

DateFormat::DateFormat(const FilterExpression &dateTime, const FilterExpression &format, Parser *parser)
    : Node{parser}
    , m_dateTime{dateTime}
    , m_format{format}
{
}

void DateFormat::render(OutputStream *stream, Context *gc) const
{
    const QVariant dtVar = m_dateTime.resolve(gc);
    const int dtVarType  = dtVar.userType();
    if (dtVarType == qMetaTypeId<SafeString>()) {
        *stream << dtVar.value<SafeString>().get();
        return;
    } else if (dtVarType == QMetaType::QString) {
        *stream << dtVar.toString();
        return;
    } else {
        switch (dtVarType) {
        case QMetaType::QDateTime:
        case QMetaType::QDate:
        case QMetaType::QTime:
            break;
        default:
            qCWarning(HBNBOTA_CUTELEE)
                << "hbnbota_dateformat can only operate on QDateTime, QDate or QTime values, but not on"
                << dtVar.metaType().name() << "(ID:" << dtVarType << "):" << dtVar;
            return;
        }
    }

    auto c = findCutelystContext(m_cutelystContext, gc);

    if (!c) {
        qCWarning(HBNBOTA_CUTELEE) << "hbnbota_dateformat can not find the Cutelyst context";
        return;
    }

    const QString format = getFormat(m_format, gc);

    switch (dtVarType) {
    case QMetaType::QDateTime:
        *stream << formatDateTime(c, dtVar, format);
        break;
    case QMetaType::QDate:
        *stream << formatDate(c, dtVar, format);
        break;
    case QMetaType::QTime:
        *stream << formatTime(c, dtVar, format);
        break;
    default:
        break;
    }
}

DateFormatVarTag::DateFormatVarTag(QObject *parent)
    : AbstractNodeFactory{parent}
{
}

Node *DateFormatVarTag::getNode(const QString &tagContent, Parser *p) const
{
    QStringList parts = smartSplit(tagContent);
    parts.removeFirst();

    if (parts.size() < 3) {
        throw Exception{TagSyntaxError, u"hbnbota_dateformat_var requires at least three arguments"_s};
    }

    FilterExpression dateTime{parts.at(0), p};

    FilterExpression format;
    if (parts.size() > 3) {
        format = FilterExpression{parts.at(1), p};
    }

    return new DateFormatVar{dateTime, format, parts.last(), p}; // NOLINT(cppcoreguidelines-owning-memory)
}

DateFormatVar::DateFormatVar(const FilterExpression &dateTime,
                             const FilterExpression &format,
                             const QString &resultName,
                             Parser *parser)
    : Node{parser}
    , m_dateTime{dateTime}
    , m_format{format}
    , m_resultName{resultName}
{
}

void DateFormatVar::render(OutputStream *stream, Context *cc) const
{
    Q_UNUSED(stream)

    const QVariant dtVar = m_dateTime.resolve(cc);
    const int dtVarType  = dtVar.userType();
    if (dtVarType == qMetaTypeId<SafeString>()) {
        cc->insert(m_resultName, dtVar.value<SafeString>().get());
        return;
    } else if (dtVarType == QMetaType::QString) {
        cc->insert(m_resultName, dtVar.toString());
        return;
    } else {
        switch (dtVarType) {
        case QMetaType::QDateTime:
        case QMetaType::QDate:
        case QMetaType::QTime:
            break;
        default:
            qCWarning(HBNBOTA_CUTELEE) << "hbnbota_dateformat_var can only operate on QDateTime, QDate and QTime, but not on"
                                       << dtVar.metaType().name() << "(ID:" << dtVarType << "):" << dtVar;
            return;
        }
    }

    auto c = findCutelystContext(m_cutelystContext, cc);
    if (!c) {
        qCWarning(HBNBOTA_CUTELEE) << "hbnbota_dateformat_var can not find the Cutelyst context";
        return;
    }

    const QString format = getFormat(m_format, cc);

    switch (dtVarType) {
    case QMetaType::QDateTime:
        cc->insert(m_resultName, formatDateTime(c, dtVar, format));
        break;
    case QMetaType::QDate:
        cc->insert(m_resultName, formatDate(c, dtVar, format));
        break;
    case QMetaType::QTime:
        cc->insert(m_resultName, formatTime(c, dtVar, format));
        break;
    default:
        break;
    }
}

#include "moc_dateformattag.cpp"
