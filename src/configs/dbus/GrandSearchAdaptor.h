// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHADAPTOR_H
#define GRANDSEARCHADAPTOR_H

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
 * Adaptor class for interface com.deepin.dde.GrandSearch
 */
class GrandSearchAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.GrandSearch")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.dde.GrandSearch\">\n"
"    <signal name=\"VisibleChanged\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"vidible\"/>\n"
"    </signal>\n"
"    <method name=\"IsVisible\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"SetVisible\">\n"
"      <arg direction=\"in\" type=\"b\" name=\"visible\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    GrandSearchAdaptor(QObject *parent);
    virtual ~GrandSearchAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool IsVisible();
    void SetVisible(bool visible);
Q_SIGNALS: // SIGNALS
    void VisibleChanged(bool vidible);
};

#endif
