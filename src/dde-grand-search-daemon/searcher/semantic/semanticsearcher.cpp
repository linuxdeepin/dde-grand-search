// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticsearcher_p.h"
#include "global/builtinsearch.h"
#include "semanticworker.h"
#include "semantichelper.h"
#include "configuration/configer.h"
#include "tools/semanticparser/semanticparser.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

SemanticSearcherPrivate::SemanticSearcherPrivate(SemanticSearcher *parent)
    : q(parent)
{
}

SemanticSearcherPrivate::~SemanticSearcherPrivate()
{
}

SemanticSearcher::SemanticSearcher(QObject *parent)
    : Searcher(parent),
      d(new SemanticSearcherPrivate(this))
{
    qCDebug(logDaemon) << "SemanticSearcher constructor";
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
    qCDebug(logDaemon) << "Checking SemanticSearcher active status";
    auto config = ConfigerIns->group(GRANDSEARCH_SEMANTIC_GROUP);
    Q_ASSERT(config);

    SemanticParser paser;
    d->m_semantic = false;
    d->m_vector = false;
    d->m_fulltext = false;

    if (config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, true)) {
        /*if (paser.connectToQueryLang(SemanticHelper::querylangServiceName())) {
            d->m_semantic  = paser.isQueryLangSupported();
        }*/
        d->m_semantic = true;
        qCDebug(logDaemon) << "Semantic analysis enabled";
    }

    //    if (config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, true)) {
    //        if (paser.connectToVector(SemanticHelper::vectorServiceName())) {
    //            d->m_vector = paser.isVectorSupported();
    //        }
    //    }

    d->m_fulltext = config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, false);

    return d->m_semantic;
}

bool SemanticSearcher::activate()
{
    qCDebug(logDaemon) << "SemanticSearcher activate called - Not supported for this searcher type";
    return false;
}

ProxyWorker *SemanticSearcher::createWorker() const
{
    qCDebug(logDaemon) << "Creating SemanticWorker";
    auto worker = new SemanticWorker(name());
    worker->setEngineState(d->m_semantic, d->m_vector, d->m_fulltext);
    qCInfo(logDaemon) << "Created SemanticWorker with engine state - Semantic:" << d->m_semantic
                      << "Vector:" << d->m_vector
                      << "Fulltext:" << d->m_fulltext;
    return worker;
}

bool SemanticSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qCWarning(logDaemon) << "Unsupported action requested:" << action;
    return false;
}
