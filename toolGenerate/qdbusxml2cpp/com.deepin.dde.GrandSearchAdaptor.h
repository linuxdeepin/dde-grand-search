/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-grand-search/src/grand-search/contacts/services/com.deepin.dde.GrandSearch.xml -a ./dde-grand-search/toolGenerate/qdbusxml2cpp/com.deepin.dde.GrandSearchAdaptor -i ./dde-grand-search/toolGenerate/qdbusxml2cpp/com.deepin.dde.GrandSearch.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef COM_DEEPIN_DDE_GRANDSEARCHADAPTOR_H
#define COM_DEEPIN_DDE_GRANDSEARCHADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "./dde-grand-search/toolGenerate/qdbusxml2cpp/com.deepin.dde.GrandSearch.h"
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
