// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchpluginadaptor.h"
#include "convertsearch.h"

#include <QDBusAbstractAdaptor>

SearchPluginAdaptor::SearchPluginAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent), m_parent(parent)
{
    setAutoRelaySignals(false);
}

SearchPluginAdaptor::~SearchPluginAdaptor() { }

QString SearchPluginAdaptor::Search(const QString &json)
{
    auto *searcher = qobject_cast<ConvertSearch *>(m_parent);
    return searcher ? searcher->search(json) : QString();
}

bool SearchPluginAdaptor::Stop(const QString &json)
{
    auto *searcher = qobject_cast<ConvertSearch *>(m_parent);
    return searcher ? searcher->stop(json) : false;
}

bool SearchPluginAdaptor::Action(const QString &json)
{
    auto *searcher = qobject_cast<ConvertSearch *>(m_parent);
    return searcher ? searcher->action(json) : false;
}
