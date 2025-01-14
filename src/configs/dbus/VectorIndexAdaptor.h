// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VECTORINDEXADAPTOR_H
#define VECTORINDEXADAPTOR_H

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
 * Adaptor class for interface org.deepin.ai.daemon.VectorIndex
 */
class VectorIndexAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.ai.daemon.VectorIndex")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.ai.daemon.VectorIndex\">\n"
"    <signal name=\"IndexStatus\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"out\" type=\"as\" name=\"files\"/>\n"
"      <arg direction=\"out\" type=\"i\" name=\"status\"/>\n"
"    </signal>\n"
"    <signal name=\"IndexDeleted\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"out\" type=\"as\" name=\"files\"/>\n"
"    </signal>\n"
"    <method name=\"Create\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"files\"/>\n"
"    </method>\n"
"    <method name=\"Delete\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"files\"/>\n"
"    </method>\n"
"    <method name=\"Search\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"query\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"topK\"/>\n"
"    </method>\n"
"    <method name=\"DocFiles\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"Enable\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"setAutoIndex\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"on\"/>\n"
"    </method>\n"
"    <method name=\"getAutoIndexStatus\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"appID\"/>\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    VectorIndexAdaptor(QObject *parent);
    virtual ~VectorIndexAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool Create(const QString &appID, const QStringList &files);
    bool Delete(const QString &appID, const QStringList &files);
    QStringList DocFiles(const QString &appID);
    bool Enable();
    QString Search(const QString &appID, const QString &query, int topK);
    QString getAutoIndexStatus(const QString &appID);
    void setAutoIndex(const QString &appID, bool on);
Q_SIGNALS: // SIGNALS
    void IndexDeleted(const QString &appID, const QStringList &files);
    void IndexStatus(const QString &appID, const QStringList &files, int status);
};

#endif
