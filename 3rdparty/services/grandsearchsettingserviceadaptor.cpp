// SPDX-FileCopyrightText: 2021 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchsettingserviceadaptor.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class GrandSearchSettingServiceAdaptor
 */

using namespace GrandSearchSetting;

GrandSearchSettingServiceAdaptor::GrandSearchSettingServiceAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

GrandSearchSettingServiceAdaptor::~GrandSearchSettingServiceAdaptor()
{
    // destructor
}

void GrandSearchSettingServiceAdaptor::Show()
{
    // handle method call com.deepin.dde.GrandSearchSetting.Show
    QMetaObject::invokeMethod(parent(), "Show");
}
