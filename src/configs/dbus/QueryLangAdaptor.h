// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERYLANGADAPTOR_H
#define QUERYLANGADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.ai.daemon.QueryLang
 */
class QueryLangAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.ai.daemon.QueryLang")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.ai.daemon.QueryLang\">\n"
"    <method name=\"Query\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"content\"/>\n"
"    </method>\n"
"    <method name=\"Enable\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    QueryLangAdaptor(QObject *parent);
    virtual ~QueryLangAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool Enable();
    QString Query(const QString &content);
Q_SIGNALS: // SIGNALS
};

#endif
