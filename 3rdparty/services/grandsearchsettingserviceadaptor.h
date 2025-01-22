// SPDX-FileCopyrightText: 2021 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHSETTINGSERVICEADAPTOR_H
#define GRANDSEARCHSETTINGSERVICEADAPTOR_H

#include "grand-search/contacts/services/grandsearchsettingservice.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QVariant;
QT_END_NAMESPACE

namespace GrandSearchSetting {

/*
 * Adaptor class for interface com.deepin.dde.GrandSearchSetting
 */
class GrandSearchSettingServiceAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.GrandSearchSetting")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.dde.GrandSearchSetting\">\n"
"    <method name=\"Show\">\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    GrandSearchSettingServiceAdaptor(QObject *parent);
    virtual ~GrandSearchSettingServiceAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void Show();
};

}

#endif
