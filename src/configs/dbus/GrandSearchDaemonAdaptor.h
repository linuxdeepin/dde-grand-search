// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHDAEMONADAPTOR_H
#define GRANDSEARCHDAEMONADAPTOR_H

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
 * Adaptor class for interface org.deepin.dde.GrandSearchDaemon
 */
class GrandSearchDaemonAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.GrandSearchDaemon")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.dde.GrandSearchDaemon\">\n"
"    <signal name=\"Matched\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"session\"/>\n"
"    </signal>\n"
"    <signal name=\"SearchCompleted\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"session\"/>\n"
"    </signal>\n"
"    <method name=\"Search\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"session\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"key\"/>\n"
"    </method>\n"
"    <method name=\"Terminate\"/>\n"
"    <method name=\"MatchedResults\">\n"
"      <arg direction=\"out\" type=\"ay\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"session\"/>\n"
"    </method>\n"
"    <method name=\"MatchedBuffer\">\n"
"      <arg direction=\"out\" type=\"ay\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"session\"/>\n"
"    </method>\n"
"    <method name=\"OpenWithPlugin\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"searcher\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"item\"/>\n"
"    </method>\n"
"    <method name=\"Configure\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"Configuration\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"SetFeedBackStrategy\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"FeedBackStrategy\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"KeepAlive\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"session\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    GrandSearchDaemonAdaptor(QObject *parent);
    virtual ~GrandSearchDaemonAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QVariantMap Configuration();
    bool Configure(const QVariantMap &in0);
    QVariantMap FeedBackStrategy();
    bool KeepAlive(const QString &session);
    QByteArray MatchedBuffer(const QString &session);
    QByteArray MatchedResults(const QString &session);
    bool OpenWithPlugin(const QString &searcher, const QString &item);
    bool Search(const QString &session, const QString &key);
    bool SetFeedBackStrategy(const QVariantMap &in0);
    void Terminate();
Q_SIGNALS: // SIGNALS
    void Matched(const QString &session);
    void SearchCompleted(const QString &session);
};

#endif
