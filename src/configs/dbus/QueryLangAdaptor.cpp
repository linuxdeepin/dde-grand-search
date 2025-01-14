// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "QueryLangAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class QueryLangAdaptor
 */

QueryLangAdaptor::QueryLangAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

QueryLangAdaptor::~QueryLangAdaptor()
{
    // destructor
}

bool QueryLangAdaptor::Enable()
{
    // handle method call org.deepin.ai.daemon.QueryLang.Enable
    bool out0;
    QMetaObject::invokeMethod(parent(), "Enable", Q_RETURN_ARG(bool, out0));
    return out0;
}

QString QueryLangAdaptor::Query(const QString &content)
{
    // handle method call org.deepin.ai.daemon.QueryLang.Query
    QString out0;
    QMetaObject::invokeMethod(parent(), "Query", Q_RETURN_ARG(QString, out0), Q_ARG(QString, content));
    return out0;
}

