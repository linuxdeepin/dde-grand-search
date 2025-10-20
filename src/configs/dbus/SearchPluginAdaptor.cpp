// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SearchPluginAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class SearchPluginAdaptor
 */

SearchPluginAdaptor::SearchPluginAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

SearchPluginAdaptor::~SearchPluginAdaptor()
{
    // destructor
}

bool SearchPluginAdaptor::Action(const QString &json)
{
    // handle method call com.deepin.dde.GrandSearch.SearchPlugin.Action
    bool out0;
    QMetaObject::invokeMethod(parent(), "Action", Q_RETURN_ARG(bool, out0), Q_ARG(QString, json));
    return out0;
}

QString SearchPluginAdaptor::Search(const QString &json)
{
    // handle method call com.deepin.dde.GrandSearch.SearchPlugin.Search
    QString out0;
    QMetaObject::invokeMethod(parent(), "Search", Q_RETURN_ARG(QString, out0), Q_ARG(QString, json));
    return out0;
}

bool SearchPluginAdaptor::Stop(const QString &json)
{
    // handle method call com.deepin.dde.GrandSearch.SearchPlugin.Stop
    bool out0;
    QMetaObject::invokeMethod(parent(), "Stop", Q_RETURN_ARG(bool, out0), Q_ARG(QString, json));
    return out0;
}

