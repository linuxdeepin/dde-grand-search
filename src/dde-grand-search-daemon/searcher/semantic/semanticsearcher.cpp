// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticsearcher_p.h"
#include "global/builtinsearch.h"
#include "semanticworker.h"
#include "semantichelper.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

using namespace GrandSearch;

SemanticSearcherPrivate::SemanticSearcherPrivate(SemanticSearcher *parent): q(parent)
{

}

SemanticSearcherPrivate::~SemanticSearcherPrivate()
{

}

SemanticSearcher::SemanticSearcher(QObject *parent)
    : Searcher(parent)
    , d(new SemanticSearcherPrivate(this))
{

}

SemanticSearcher::~SemanticSearcher()
{
    delete d;
    d = nullptr;
}

QString SemanticSearcher::name() const
{
    return GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC;
}

bool SemanticSearcher::isActive() const
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(SemanticHelper::serviceName()).value();
}

bool SemanticSearcher::activate()
{
    return false;
}

ProxyWorker *SemanticSearcher::createWorker() const
{
    return new SemanticWorker(name(), SemanticHelper::serviceName());
}

bool SemanticSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qWarning() << "no such action:" << action << ".";
    return false;
}
