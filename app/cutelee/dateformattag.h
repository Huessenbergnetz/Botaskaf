/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef HBNBOTA_DATEFORMATTAG_H
#define HBNBOTA_DATEFORMATTAG_H

#include <cutelee/node.h>

using namespace Cutelee;

/*!
 * \brief Provides the Cutelee hbnbota_dateformat tag.
 *
 * The \c hbnbota_dateformat tag can be used to format a QDateTime, QDate or QTime according to
 * Zumftmeistar user settings. The generated string will be localized accoring to the user’s
 * QLocale set to Cutelyst::Context::locale().
 *
 * If the input value is of type QDateTime, it is expected to be an UTC datetime. It will then
 * be converted into the user’s time zone.
 *
 * The \c hbnbota_dateformat tag provides an optional \a format argument that either recognizes the
 * keywords “short“ and “long” or a format string as used by QDateTime::toString(). “short“ and
 * “long“ will create strings according to <A
 * HREF="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/DateTimeFormat">JavaScript’s
 * Intl.DateTimeFormat</A>. If the \a format argument is omitted, “short“ will be used.
 *
 * <H3>%QDateTime short format</H3>
 *
 * The “short“ format for a QDateTime will create a localized string according to JavaScript’s
 * Intl.DateTimeFormat with options <TT>dateStyle: short</TT> and <TT>timeStyle: short</TT>.
 * The %Qt format string used for the \a en_US locale is <TT>M/d/yy, h:mm AP</TT>.
 *
 * <H3>%QDateTime long format</H3>
 *
 * The “long” format for a QDateTime will create a localized string according to JavaScript’s
 * Intl.DateTimeFormat with options <TT>dateStyle: long</TT> and <TT>timeStyle: meidum</TT>.
 * The %Qt format string used for the \a en_US locale is <TT>MMMM d, yyyy 'at' h:mm:ss AP</TT>.
 *
 * <H3>%QDate short format</H3>
 *
 * The “short“ format for a QDate will create a localized string according to JavaScript’s
 * Intl.DateTimeFormat with option <TT>dateStyle: short</TT>. The %Qt format string used for
 * the \a en_US locale is <TT>M/d/yy</TT>.
 *
 * <H3>%QDate long format</H3>
 *
 * The “long“ format for a QDate will create a localized string according to JavaScript’s
 * Intl.DateTimeFormat with option <TT>dateStyle: long</TT>. The %Qt format string used for
 * the \a en_US locale is <TT>MMMM d, yyyy</TT>.
 *
 * <H3>%QTime short format</H3>
 *
 * The “short“ format for a QTime will create a localized string according to JavaScript’s
 * Intl.DateTimeFormat with option <TT>timeStyle: short</TT>. The %Qt format string used for
 * the \a en_US locale is <TT>h:mm AP</TT>.
 *
 * <H3>%QTime long format</H3>
 *
 * The “long“ format for a QTime will create a localized string according to JavaScript’s
 * Intl.DateTimeFormat with option <TT>timeStyle: medium</TT>. The %Qt format string used for
 * the \a en_US locale is <TT>h:mm:ss AP</TT>.
 *
 * <H3>Examples</H3>
 *
 * \code
 * {% hbnbota_dateformat myDateTime %}
 *
 * {% hbnbota_dateformat myDateTime "long" %}
 *
 * {% hbnbota_dateformat myDateTime "dd.MM.yyyy hh:mm" %}
 * \endcode
 *
 * \sa DateFormatVarTag
 */
class DateFormatTag final : public AbstractNodeFactory
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new %DateFormatTag object with the given \a parent.
     */
    explicit DateFormatTag(QObject *parent = nullptr);
    /*!
     * \brief Destroys the %DateFormatTag object.
     */
    ~DateFormatTag() override = default;

    /*!
     * \brief Returns the Node with the given \a tagContent.
     */
    Node *getNode(const QString &tagContent, Parser *p) const override;

private:
    Q_DISABLE_COPY(DateFormatTag)
};

class DateFormat final : public Node // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
public:
    explicit DateFormat(const FilterExpression &dateTime, const FilterExpression &format, Parser *parser = nullptr);
    ~DateFormat() override = default;

    void render(OutputStream *stream, Context *gc) const override;

private:
    mutable QString m_cutelystContext{QStringLiteral("c")};
    FilterExpression m_dateTime;
    FilterExpression m_format;

    Q_DISABLE_COPY(DateFormat)
};

/*!
 * \brief Provides the Cutelee hbnbota_dateformat_var tag.
 *
 * The \c hbnbota_dateformat_var tag can be used to format a QDateTime, QDate or QTime according to
 * Zumftmeistar user settings and store it into a new Cutelee context variable. The generated string
 * will be localized accoring to the user’s QLocale set to Cutelyst::Context::locale().
 *
 * If the input value is of type QDateTime, it is expected to be an UTC datetime. It will then
 * be converted into the user’s time zone.
 *
 * The \c hbnbota_dateformat_var tag provides an optional \a format argument that either recognizes the
 * keywords “short“ and “long” or a format string as used by QDateTime::toString(). “short“ and
 * “long“ will create strings according to <A
 * HREF="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/DateTimeFormat">JavaScript’s
 * Intl.DateTimeFormat</A>. If the \a format argument is omitted, “short“ will be used.
 *
 * Have a look at the documentation of the \link DateFormatTag hbnbota_dateformat tag\endlink to learn
 * more about the format argument options.
 *
 * <H3>Examples</H3>
 *
 * \code
 * {% hbnbota_dateformat_var myDateTime as myFormattedDateTime %}
 * {{ myFormattedDateTime }}
 *
 * {% hbnbota_dateformat_var myDateTime "long" as myFormattedDateTime %}
 * {{ myFormattedDateTime }}
 *
 * {% hbnbota_dateformat_var myDateTime "dd.MM.yyyy hh:mm" as myFormattedDateTime %}
 * {{ myFormattedDateTime }}
 * \endcode
 *
 * \sa DateFormatTag
 */
class DateFormatVarTag final : public AbstractNodeFactory
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new %DateFormatVarTag object with the given \a parent.
     */
    explicit DateFormatVarTag(QObject *parent = nullptr);
    /*!
     * \brief Destroys the %DateFormatVarTag object.
     */
    ~DateFormatVarTag() override = default;

    /*!
     * \brief Returns the Node with the given \a tagContent.
     */
    Node *getNode(const QString &tagContent, Parser *p) const override;

private:
    Q_DISABLE_COPY(DateFormatVarTag)
};

class DateFormatVar final : public Node // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
public:
    explicit DateFormatVar(const FilterExpression &dateTime,
                           const FilterExpression &format,
                           const QString &resultName,
                           Parser *parser = nullptr);
    ~DateFormatVar() override = default;

    void render(OutputStream *stream, Context *cc) const override;

private:
    mutable QString m_cutelystContext{QStringLiteral("c")};
    FilterExpression m_dateTime;
    FilterExpression m_format;
    QString m_resultName;

    Q_DISABLE_COPY(DateFormatVar)
};

#endif // HBNBOTA_DATEFORMATTAG_H
