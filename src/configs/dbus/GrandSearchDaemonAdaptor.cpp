// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GrandSearchDaemonAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class GrandSearchDaemonAdaptor
 */

GrandSearchDaemonAdaptor::GrandSearchDaemonAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

GrandSearchDaemonAdaptor::~GrandSearchDaemonAdaptor()
{
    // destructor
}

QVariantMap GrandSearchDaemonAdaptor::Configuration()
{
    // handle method call org.deepin.dde.GrandSearchDaemon.Configuration
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "Configuration", Q_RETURN_ARG(QVariantMap, out0));
    return out0;
}

bool GrandSearchDaemonAdaptor::Configure(const QVariantMap &in0)
{
    // handle method call org.deepin.dde.GrandSearchDaemon.Configure
    bool out0;
    QMetaObject::invokeMethod(parent(), "Configure", Q_RETURN_ARG(bool, out0), Q_ARG(QVariantMap, in0));
    return out0;
}

QVariantMap GrandSearchDaemonAdaptor::FeedBackStrategy()
{
    // handle method call org.deepin.dde.GrandSearchDaemon.FeedBackStrategy
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "FeedBackStrategy", Q_RETURN_ARG(QVariantMap, out0));
    return out0;
}

bool GrandSearchDaemonAdaptor::KeepAlive(const QString &session)
{
    // handle method call org.deepin.dde.GrandSearchDaemon.KeepAlive
    bool out0;
    QMetaObject::invokeMethod(parent(), "KeepAlive", Q_RETURN_ARG(bool, out0), Q_ARG(QString, session));
    return out0;
}

QByteArray GrandSearchDaemonAdaptor::MatchedBuffer(const QString &session)
{
    // handle method call org.deepin.dde.GrandSearchDaemon.MatchedBuffer
    QByteArray out0;
    QMetaObject::invokeMethod(parent(), "MatchedBuffer", Q_RETURN_ARG(QByteArray, out0), Q_ARG(QString, session));
    return out0;
}

QByteArray GrandSearchDaemonAdaptor::MatchedResults(const QString &session)
{
    // handle method call org.deepin.dde.GrandSearchDaemon.MatchedResults
    QByteArray out0;
    QMetaObject::invokeMethod(parent(), "MatchedResults", Q_RETURN_ARG(QByteArray, out0), Q_ARG(QString, session));
    return out0;
}

bool GrandSearchDaemonAdaptor::OpenWithPlugin(const QString &searcher, const QString &item)
{
    // handle method call org.deepin.dde.GrandSearchDaemon.OpenWithPlugin
    bool out0;
    QMetaObject::invokeMethod(parent(), "OpenWithPlugin", Q_RETURN_ARG(bool, out0), Q_ARG(QString, searcher), Q_ARG(QString, item));
    return out0;
}

bool GrandSearchDaemonAdaptor::Search(const QString &session, const QString &key)
{
    // handle method call org.deepin.dde.GrandSearchDaemon.Search
    bool out0;
    QMetaObject::invokeMethod(parent(), "Search", Q_RETURN_ARG(bool, out0), Q_ARG(QString, session), Q_ARG(QString, key));
    return out0;
}

bool GrandSearchDaemonAdaptor::SetFeedBackStrategy(const QVariantMap &in0)
{
    // handle method call org.deepin.dde.GrandSearchDaemon.SetFeedBackStrategy
    bool out0;
    QMetaObject::invokeMethod(parent(), "SetFeedBackStrategy", Q_RETURN_ARG(bool, out0), Q_ARG(QVariantMap, in0));
    return out0;
}

void GrandSearchDaemonAdaptor::Terminate()
{
    // handle method call org.deepin.dde.GrandSearchDaemon.Terminate
    QMetaObject::invokeMethod(parent(), "Terminate");
}

