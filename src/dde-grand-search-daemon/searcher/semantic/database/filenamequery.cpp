// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenamequery.h"
#include "filenamequery_p.h"
#include "global/builtinsearch.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;
DFM_SEARCH_USE_NS

FileNameQueryPrivate::FileNameQueryPrivate()
{
}

void FileNameQueryPrivate::searchByDFMSearch(PushItemCallBack callBack, void *pdata)
{
    qCDebug(logDaemon) << "Starting DFM search with" << m_entity.size() << "entities";
    for (const SemanticEntity &entity : std::as_const(m_entity)) {
        if (m_handler && m_handler->isResultLimit()) {
            qCDebug(logDaemon) << "Search terminated - Result limit reached";
            return;
        }

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
        bool terminated = false;
        engine->searchWithCallback(query, [this, &terminated, &entity, callBack, pdata](const SearchResult &file) -> bool {
            auto ret = processSearchResult(file.path(), entity, callBack, pdata);
            if (ret == Terminated) {
                terminated = true;
            }
            return terminated;
        });

        if (terminated)
            break;
    }

    callBack(std::move(m_resultItems), pdata);
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
    } else {
        QList<FileSearchUtils::Group> groupList = { FileSearchUtils::Group::File };
        fileNameOptions.setFileTypes(FileSearchUtils::buildDFMSearchFileTypes(groupList));

        if (!entity.suffix.isEmpty() && entity.isContainType) {
            fileNameOptions.setFileExtensions({ entity.suffix });
        }
    }
}

FileNameQueryPrivate::ProcessResult FileNameQueryPrivate::processSearchResult(const QString &result,
                                                                              const SemanticEntity &entity,
                                                                              PushItemCallBack callBack,
                                                                              void *pdata)
{
    // 检查是否需要推送数据与中断
    if (timeToPush()) {
        auto it = std::move(m_resultItems);
        bool ret = callBack(it, pdata);
        // 有数据放入，更新时间
        if (!it.isEmpty())
            m_lastPush = m_time.elapsed();

        // 中断
        if (!ret)
            return Terminated;
    }

    if (!checkResultValid(result, entity))
        return Invalid;

    m_count++;
    if (m_handler) {
        m_handler->appendTo(result, m_resultItems);
        const auto fileName = result.mid(result.lastIndexOf('/') + 1);
        m_handler->setItemWeight(result, m_handler->itemWeight(result) + calcItemWeight(fileName));
        if (m_handler->isResultLimit() || m_count >= 100) {
            qCDebug(logDaemon) << "Search terminated - Result limit reached (count:" << m_count << ")";
            return Terminated;
        }
    } else {
        auto item = FileSearchUtils::packItem(result, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
        m_resultItems[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
    }

    return Success;
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
    
    // 检查文件名索引目录是否可用，如果不可用则回退到实时搜索
    if (DFMSEARCH::Global::isFileNameIndexDirectoryAvailable()) {
        qCDebug(logDaemon) << "Using indexed search method for semantic query";
        options.setSearchMethod(SearchMethod::Indexed);
    } else {
        qCWarning(logDaemon) << "File name index directory is not available, falling back to realtime search for semantic query";
        options.setSearchMethod(SearchMethod::Realtime);
    }
    
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
    qCInfo(logDaemon) << "Starting query with" << d->m_entity.size() << "entities";
    d->m_time.start();
    d->searchByDFMSearch(callBack, pdata);
    qCInfo(logDaemon) << "Search completed - Time elapsed:" << d->m_time.elapsed()
                      << "ms, Results found:" << d->m_count;
}

void FileNameQuery::setEntity(const QList<SemanticEntity> &entity)
{
    d->m_entity.clear();

    for (const SemanticEntity &e : entity) {
        if (!e.album.isEmpty() || !e.author.isEmpty() || !e.duration.isEmpty()
            || !e.resolution.isEmpty()) {
            qCDebug(logDaemon) << "Skipping entity with media metadata:" << e.keys;
            continue;
        }
        d->m_entity.append(e);
    }
}

void FileNameQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}
