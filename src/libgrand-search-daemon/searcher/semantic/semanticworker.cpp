// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticworker_p.h"
#include "global/builtinsearch.h"
#include "semanticparser/semanticparser.h"
#include "semantichelper.h"
#include "fileresultshandler.h"

#include "database/anythingquery.h"
#include "database/fulltextquery.h"
#include "database/featurequery.h"

#include <QDebug>
#include <QtConcurrent>

using namespace GrandSearch;

#define checkRuning() \
if (d->m_status.loadAcquire() != ProxyWorker::Runing) \
    return false

SemanticWorkerPrivate::SemanticWorkerPrivate(SemanticWorker *parent) : q(parent)
{

}

SemanticWorkerPrivate::~SemanticWorkerPrivate()
{

}

void SemanticWorkerPrivate::tryNotify()
{
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > 100) {
        m_lastEmit = cur;
        qDebug() << "unearthed, current spend:" << cur;
        emit q->unearthed(q);
    }
}

bool SemanticWorkerPrivate::pushItem(const MatchedItemMap &items, void *ptr)
{
    SemanticWorkerPrivate *d =  static_cast<SemanticWorkerPrivate *>(ptr);
    if (d->m_status.loadAcquire() == ProxyWorker::Terminated)
        return false;

    // lock and append item
    bool hasItem = false;
    {
        QMutexLocker lk(&d->m_mtx);
        for(auto it = items.begin(); it != items.end(); ++it) {
            if (it.value().isEmpty())
                continue;
            d->m_items[it.key()].append(it.value());
        }
        hasItem = !d->m_items.isEmpty();
    }

    if (hasItem)
        d->tryNotify();

    return true;
}

void SemanticWorkerPrivate::run(const QueryFunction &func)
{
    func.function(func.object, &SemanticWorkerPrivate::pushItem, func.worker);
}

SemanticWorker::SemanticWorker(const QString &name, const QString &service, QObject *parent)
    : ProxyWorker(name, parent)
    , d(new SemanticWorkerPrivate(this))
{
    d->m_serviceName = service;
}

SemanticWorker::~SemanticWorker()
{
    delete d;
    d = nullptr;
}

void SemanticWorker::setContext(const QString &context)
{
    d->m_context = context;
}

bool SemanticWorker::isAsync() const
{
    return false;
}

bool SemanticWorker::working(void *context)
{
    if (!d->m_status.testAndSetRelease(Ready, Runing))
        return false;

    // 等待2秒确认无输入后再执行。
    int count = 10;
    while (count > 0) {
        checkRuning();
        QThread::msleep(200);
        count--;
    }

    SemanticEntity entity;
    // get entity
    {
        SemanticParser parser;;
        if (!parser.connectToHost(d->m_serviceName))
            return false;

        checkRuning();

        QString ret = parser.analyze(d->m_context);
        qDebug() << "get reply" << ret;

        checkRuning();

        if (!SemanticHelper::entityFromJson(ret, entity)) {
            qWarning() << "invild entity json.";
            return false;
        }

        // 无效的搜索信息
        if (entity.keys.isEmpty() && entity.types.isEmpty() && entity.times.isEmpty()) {
            qDebug() << "invaild entity to search.";
            return false;
        }
    }

    FileResultsHandler fileHandler;
    QList<SemanticWorkerPrivate::QueryFunction> querys;

    AnythingQuery anything;
    {
        SemanticWorkerPrivate::QueryFunction func = {&anything, &AnythingQuery::run, d};
        querys.append(func);
        anything.setEntity(entity);
        anything.setFileHandler(&fileHandler);
    }

    FullTextQuery fuletext;
    {
        SemanticWorkerPrivate::QueryFunction func = {&fuletext, &FullTextQuery::run, d};
        querys.append(func);
        fuletext.setEntity(entity);
        fuletext.setFileHandler(&fileHandler);
    }

    FeatureQuery feature;
    {
        SemanticWorkerPrivate::QueryFunction func = {&feature, &FeatureQuery::run, d};
        querys.append(func);
        feature.setEntity(entity);
        feature.setFileHandler(&fileHandler);
    }

    checkRuning();

    d->m_time.start();
    auto future = QtConcurrent::map(querys, &SemanticWorkerPrivate::run);
    future.waitForFinished();

    // 发送遗留的数据
    if (!d->m_items.isEmpty())
        emit unearthed(this);

    return true;
}

void SemanticWorker::terminate()
{
    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status SemanticWorker::status()
{
    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool SemanticWorker::hasItem() const
{
    QMutexLocker lk(&d->m_mtx);
    return !d->m_items.isEmpty();
}

MatchedItemMap SemanticWorker::takeAll()
{
    QMutexLocker lk(&d->m_mtx);
    MatchedItemMap items = std::move(d->m_items);
    lk.unlock();

    return items;
}
