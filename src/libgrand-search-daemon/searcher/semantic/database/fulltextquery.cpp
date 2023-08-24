// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fulltextquery_p.h"

#include "utils/specialtools.h"
#include "luceneengine/fulltextengine.h"
#include "global/searchhelper.h"
#include "global/builtinsearch.h"

using namespace GrandSearch;

FullTextQueryPrivate::FullTextQueryPrivate(FullTextQuery *qq) : q(qq)
{

}

bool FullTextQueryPrivate::processResult(const QString &file, void *pdata)
{
    FullTextQueryPrivate::Context *context = static_cast<FullTextQueryPrivate::Context *>(pdata);
    Q_ASSERT(context);

    auto d = context->query->d;

    // 检查是否需要推送数据与中断
    if (d->timeToPush()) {
        auto it = std::move(d->m_results);
        bool ret = context->callBack(it, context->callBackData);
        // 有数据放入，更新时间
        if (!it.isEmpty())
            d->m_lastPush = d->m_time.elapsed();

        // 中断
        if (!ret)
            return false;
    }

   QFileInfo info(file);

   // 检查后缀
   if (!context->suffix.isEmpty() && !context->suffix.contains(info.suffix()))
       return true;

   // 检查时间
   if (!SemanticHelper::isMatchTime(info.lastModified().toSecsSinceEpoch(), context->times))
       return true;

   // 过滤文管设置的隐藏文件
   QHash<QString, QSet<QString>> hiddenFilters;
   if (SpecialTools::isHiddenFile(file, hiddenFilters, QDir::homePath()))
       return true;

   if (d->m_handler) {
       d->m_handler->appendTo(file, d->m_results);
       if (d->m_handler->isResultLimit())
           return false;
   } else {
       auto item = FileSearchUtils::packItem(file, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
       auto group = FileSearchUtils::getGroupByName(file);
       d->m_results[FileSearchUtils::groupKey(group)].append(item);
   }

   return true;
}

bool FullTextQueryPrivate::timeToPush() const
{
    return (m_time.elapsed() - m_lastPush) > 100;
}

FullTextQuery::FullTextQuery(QObject *parent)
    : QObject(parent)
    , d(new FullTextQueryPrivate(this))
{

}

FullTextQuery::~FullTextQuery()
{
    delete d;
    d = nullptr;
}

void FullTextQuery::run(void *ptr, PushItemCallBack callBack, void *pdata)
{
    qDebug() << "query by full text";
    Q_ASSERT(callBack);

    FullTextQuery *self = static_cast<FullTextQuery *>(ptr);
    Q_ASSERT(self);

    auto d = self->d;

    FullTextEngine engine;
    if (!engine.init(d->indexStorePath()))
        return;

    FullTextQueryPrivate::Context context;
    context.query = self;
    context.callBack = callBack;
    context.callBackData = pdata;
    context.suffix = SemanticHelper::typeTosuffix(d->m_entity.types);
    context.times = d->m_entity.times;

    QString path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();

    d->m_time.start();
    engine.query(path, d->m_entity.keys, &FullTextQueryPrivate::processResult, &context);

    callBack(d->m_results, pdata);
}

void FullTextQuery::setEntity(const SemanticEntity &entity)
{
    d->m_entity = entity;
}

void FullTextQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}

