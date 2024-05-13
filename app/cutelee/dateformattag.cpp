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

QString formatDateTime(Cutelyst::Context *c, const QVariant &dtVar, const QString &format)
{
    QDateTime dt = dtVar.toDateTime();

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
