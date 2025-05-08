// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenamequery.h"
#include "filenamequery_p.h"
#include "global/builtinsearch.h"

using namespace GrandSearch;
DFM_SEARCH_USE_NS

#define MAX_SEARCH_NUM_TOTAL 10000

FileNameQueryPrivate::FileNameQueryPrivate()
{
}

void FileNameQueryPrivate::searchByDFMSearch(PushItemCallBack callBack, void *pdata)
{
    for (const SemanticEntity &entity : m_entity) {
        if (m_handler && m_handler->isResultLimit())
            break;

        QObject holder;
        SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);

        // 创建搜索选项
        SearchOptions options = createSearchOptions(entity);
        // 创建搜索查询
        SearchQuery query = createSearchQuery(entity);

        // 配置文件名选项
        FileNameOptionsAPI fileNameOptions(options);
        configureFileNameOptions(fileNameOptions, query, entity);

        // 设置搜索选项
        engine->setSearchOptions(options);

        // 执行搜索并处理结果
        const SearchResultExpected &result = engine->searchSync(query);
        if (!processSearchResults(result, entity, callBack, pdata))
            return;
    }

    callBack(m_resultItems, pdata);
}

void FileNameQueryPrivate::configureFileNameOptions(FileNameOptionsAPI &fileNameOptions, const SearchQuery &query, const SemanticEntity &entity) const
{
    if (!entity.types.isEmpty()) {
        QList<FileSearchUtils::Group> groupList;
        std::transform(entity.types.begin(), entity.types.end(), std::back_inserter(groupList),
                       [](const QString &group) {
                           return FileSearchUtils::getGroupByGroupName(group);
                       });
        fileNameOptions.setFileTypes(FileSearchUtils::buildDFMSearchFileTypes(groupList));
    } else if (!entity.suffix.isEmpty() && entity.isContainType) {
        fileNameOptions.setFileExtensions({ entity.suffix });
    }
}

bool FileNameQueryPrivate::processSearchResults(const SearchResultExpected &result,
                                                const SemanticEntity &entity,
                                                PushItemCallBack callBack,
                                                void *pdata)
{
    if (!result.hasValue()) {
        qWarning() << "DFMSearch failed for key: " << entity.keys << result.error().message();
        return false;
    }

    for (const auto &file : result.value()) {
        // 检查是否需要推送数据与中断
        if (timeToPush()) {
            auto it = std::move(m_resultItems);
            bool ret = callBack(it, pdata);
            // 有数据放入，更新时间
            if (!it.isEmpty())
                m_lastPush = m_time.elapsed();

            // 中断
            if (!ret)
                return false;
        }

        const auto result = file.path();
        if (!checkResultValid(result, entity))
            continue;

        m_count++;
        if (m_handler) {
            m_handler->appendTo(result, m_resultItems);
            const auto fileName = result.mid(result.lastIndexOf('/') + 1);
            m_handler->setItemWeight(result, m_handler->itemWeight(result) + calcItemWeight(fileName));
            if (m_handler->isResultLimit() || m_count >= 100)
                break;
        } else {
            auto item = FileSearchUtils::packItem(result, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
            m_resultItems[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
        }
    }

    return true;
}

bool FileNameQueryPrivate::checkResultValid(const QString &result, const SemanticEntity &entity)
{
    if (!entity.partPath.isEmpty()) {
        bool isMatch = result.contains(entity.partPath);
        if ((!entity.isContainPath && isMatch) || (entity.isContainPath && !isMatch)) {
            return false;
        }
    }

    QFileInfo info(result);
    if (!entity.isContainType && info.suffix() == entity.suffix)
        return false;

    if (!SemanticHelper::isMatchTime(info.lastModified().toSecsSinceEpoch(), entity.times)
        && !SemanticHelper::isMatchTime(info.birthTime().toSecsSinceEpoch(), entity.times))
        return false;

    // 检查文件大小
    if (!entity.fileCompType.isEmpty()) {
        if (entity.fileCompType == ">") {
            if (!(info.size() > entity.fileSize)) {
                return false;
            }
        } else if (entity.fileCompType == "<") {
            if (!(info.size() < entity.fileSize)) {
                return false;
            }
        } else if (entity.fileCompType == "!=") {
            if (!(info.size() != entity.fileSize)) {
                return false;
            }
        } else {
            if (!(info.size() == entity.fileSize)) {
                return false;
            }
        }
    }

    return true;
}

SearchQuery FileNameQueryPrivate::createSearchQuery(const SemanticEntity &entity) const
{
    const auto &keyList = entity.keys;
    SearchQuery query;

    if (keyList.size() >= 2) {
        query = SearchFactory::createQuery(keyList, SearchQuery::Type::Boolean);
        query.setBooleanOperator(SearchQuery::BooleanOperator::OR);
    } else if (keyList.size() == 1) {
        query = SearchFactory::createQuery(keyList.first(), SearchQuery::Type::Simple);
    } else {
        query = SearchFactory::createQuery("", SearchQuery::Type::Simple);
    }

    return query;
}

SearchOptions FileNameQueryPrivate::createSearchOptions(const SemanticEntity &entity) const
{
    SearchOptions options;
    options.setSearchPath(QDir::rootPath());
    options.setSearchMethod(SearchMethod::Indexed);
    options.setMaxResults(MAX_SEARCH_NUM_TOTAL);
    return options;
}

bool FileNameQueryPrivate::timeToPush() const
{
    return (m_time.elapsed() - m_lastPush) > 100;
}

double FileNameQueryPrivate::calcItemWeight(const QString &name)
{
    double w = 0;

    QSet<QString> mergedKeys;
    for (const SemanticEntity &e : m_entity) {
        auto keys = e.keys;
        for (const QString &key : keys)
            mergedKeys.insert(key);
    }

    for (const QString &key : mergedKeys) {
        if (name.contains(key, Qt::CaseInsensitive))
            w += 15;
    }

    return w;
}

FileNameQuery::FileNameQuery(QObject *parent)
    : QObject(parent),
      d(new FileNameQueryPrivate())
{
}

FileNameQuery::~FileNameQuery()
{
    delete d;
}

void FileNameQuery::run(void *ptr, PushItemCallBack callBack, void *pdata)
{
    Q_ASSERT(callBack);

    FileNameQuery *self = static_cast<FileNameQuery *>(ptr);
    Q_ASSERT(self);

    auto d = self->d;
    qDebug() << "query by dfm search" << d->m_entity.size();
    d->m_time.start();
    d->searchByDFMSearch(callBack, pdata);
    qDebug() << "deepin anything is finished spend:" << d->m_time.elapsed() << "found:" << d->m_count;
}

void FileNameQuery::setEntity(const QList<SemanticEntity> &entity)
{
    d->m_entity.clear();

    for (const SemanticEntity &e : entity) {
        if (!e.album.isEmpty() || !e.author.isEmpty() || !e.duration.isEmpty()
            || !e.resolution.isEmpty())
            continue;
        d->m_entity.append(e);
    }
}

void FileNameQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}
