/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-grand-search/3rdparty/interfaces/org.deepin.ai.daemon.QueryLang.xml -a ./dde-grand-search/toolGenerate/qdbusxml2cpp/org.deepin.ai.daemon.QueryLangAdaptor -i ./dde-grand-search/toolGenerate/qdbusxml2cpp/org.deepin.ai.daemon.QueryLang.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "./dde-grand-search/toolGenerate/qdbusxml2cpp/org.deepin.ai.daemon.QueryLangAdaptor.h"
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

