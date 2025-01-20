// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "VectorIndexAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class VectorIndexAdaptor
 */

VectorIndexAdaptor::VectorIndexAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

VectorIndexAdaptor::~VectorIndexAdaptor()
{
    // destructor
}

bool VectorIndexAdaptor::Create(const QString &appID, const QStringList &files)
{
    // handle method call org.deepin.ai.daemon.VectorIndex.Create
    bool out0;
    QMetaObject::invokeMethod(parent(), "Create", Q_RETURN_ARG(bool, out0), Q_ARG(QString, appID), Q_ARG(QStringList, files));
    return out0;
}

bool VectorIndexAdaptor::Delete(const QString &appID, const QStringList &files)
{
    // handle method call org.deepin.ai.daemon.VectorIndex.Delete
    bool out0;
    QMetaObject::invokeMethod(parent(), "Delete", Q_RETURN_ARG(bool, out0), Q_ARG(QString, appID), Q_ARG(QStringList, files));
    return out0;
}

QStringList VectorIndexAdaptor::DocFiles(const QString &appID)
{
    // handle method call org.deepin.ai.daemon.VectorIndex.DocFiles
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "DocFiles", Q_RETURN_ARG(QStringList, out0), Q_ARG(QString, appID));
    return out0;
}

bool VectorIndexAdaptor::Enable()
{
    // handle method call org.deepin.ai.daemon.VectorIndex.Enable
    bool out0;
    QMetaObject::invokeMethod(parent(), "Enable", Q_RETURN_ARG(bool, out0));
    return out0;
}

QString VectorIndexAdaptor::Search(const QString &appID, const QString &query, int topK)
{
    // handle method call org.deepin.ai.daemon.VectorIndex.Search
    QString out0;
    QMetaObject::invokeMethod(parent(), "Search", Q_RETURN_ARG(QString, out0), Q_ARG(QString, appID), Q_ARG(QString, query), Q_ARG(int, topK));
    return out0;
}

QString VectorIndexAdaptor::getAutoIndexStatus(const QString &appID)
{
    // handle method call org.deepin.ai.daemon.VectorIndex.getAutoIndexStatus
    QString out0;
    QMetaObject::invokeMethod(parent(), "getAutoIndexStatus", Q_RETURN_ARG(QString, out0), Q_ARG(QString, appID));
    return out0;
}

void VectorIndexAdaptor::setAutoIndex(const QString &appID, bool on)
{
    // handle method call org.deepin.ai.daemon.VectorIndex.setAutoIndex
    QMetaObject::invokeMethod(parent(), "setAutoIndex", Q_ARG(QString, appID), Q_ARG(bool, on));
}

