// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchpluginadaptor.h"
#include "calculatorsearch.h"

#include <QMetaObject>
#include <QDBusMessage>

SearchPluginAdaptor::SearchPluginAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent), m_parent(parent)
{
    setAutoRelaySignals(false);
}

SearchPluginAdaptor::~SearchPluginAdaptor()
{
}

QString SearchPluginAdaptor::Search(const QString &json)
{
    CalculatorSearch *searcher = qobject_cast<CalculatorSearch *>(m_parent);
    if (searcher)
        return searcher->search(json);
    return QString();
}

bool SearchPluginAdaptor::Stop(const QString &json)
{
    CalculatorSearch *searcher = qobject_cast<CalculatorSearch *>(m_parent);
    if (searcher)
        return searcher->stop(json);
    return false;
}

bool SearchPluginAdaptor::Action(const QString &json)
{
    CalculatorSearch *searcher = qobject_cast<CalculatorSearch *>(m_parent);
    if (searcher)
        return searcher->action(json);
    return false;
}
