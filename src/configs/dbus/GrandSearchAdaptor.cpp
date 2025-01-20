// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GrandSearchAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class GrandSearchAdaptor
 */

GrandSearchAdaptor::GrandSearchAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

GrandSearchAdaptor::~GrandSearchAdaptor()
{
    // destructor
}

bool GrandSearchAdaptor::IsVisible()
{
    // handle method call com.deepin.dde.GrandSearch.IsVisible
    bool out0;
    QMetaObject::invokeMethod(parent(), "IsVisible", Q_RETURN_ARG(bool, out0));
    return out0;
}

void GrandSearchAdaptor::SetVisible(bool visible)
{
    // handle method call com.deepin.dde.GrandSearch.SetVisible
    QMetaObject::invokeMethod(parent(), "SetVisible", Q_ARG(bool, visible));
}

