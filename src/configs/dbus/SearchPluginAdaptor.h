// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHPLUGINADAPTOR_H
#define SEARCHPLUGINADAPTOR_H

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
 * Adaptor class for interface com.deepin.dde.GrandSearch.SearchPlugin
 */
class SearchPluginAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.GrandSearch.SearchPlugin")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.dde.GrandSearch.SearchPlugin\">\n"
"    <method name=\"Search\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"json\"/>\n"
"    </method>\n"
"    <method name=\"Action\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"json\"/>\n"
"    </method>\n"
"    <method name=\"Stop\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"json\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    SearchPluginAdaptor(QObject *parent);
    virtual ~SearchPluginAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool Action(const QString &json);
    QString Search(const QString &json);
    bool Stop(const QString &json);
Q_SIGNALS: // SIGNALS
};

#endif
