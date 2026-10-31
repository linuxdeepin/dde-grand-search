// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maincontroller.h"
#include "maincontroller_p.h"
#include "configuration/configer.h"
#include "global/searchhelper.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

// 10分钟后进入休眠模式
#define DORMANT_INTERVAL 10 * 60 * 1000

MainControllerPrivate::MainControllerPrivate(MainController *parent)
    : QObject(parent),
      q(parent)
{
    qCDebug(logDaemon) << "MainControllerPrivate constructor started";

    connect(&m_dormancy, &QTimer::timeout, this, &MainControllerPrivate::dormancy);
    m_dormancy.setSingleShot(true);
    m_dormancy.setInterval(DORMANT_INTERVAL);

    qCDebug(logDaemon) << "MainControllerPrivate initialized with dormancy interval:" << DORMANT_INTERVAL << "ms";
}

MainControllerPrivate::~MainControllerPrivate()
{
    qCDebug(logDaemon) << "MainControllerPrivate destructor started";

    if (m_currentTask) {
        qCDebug(logDaemon) << "Cleaning up current task during destruction - Task ID:" << m_currentTask->taskID();
        m_currentTask->stop();
        m_currentTask->deleteSelf();
        m_currentTask = nullptr;
    }

    qCDebug(logDaemon) << "MainControllerPrivate destroyed";
}

void MainControllerPrivate::buildWorker(TaskCommander *task, const QSet<QString> &blankList)
{
    Q_ASSERT(task);
    Q_ASSERT(m_searchers);

    auto searchers = m_searchers->searchers();
    qCDebug(logDaemon) << "Available searchers count:" << searchers.size();

    // 搜索项是否启用
    auto config = ConfigerIns->group(GRANDSEARCH_PREF_SEARCHERENABLED);
    Q_ASSERT(config);

    QStringList searcherData;
    {
        // 类型过滤语法（doc:/pic:/video:/audio:/zip:/exe:/pdf:/ext:xxx）：
        // 只要文本含冒号（半角 ':' 或全角 '：'）就由 FileSearchUtils 本地解析——
        // 兼容两种冒号、多关键字 AND、排除词、/目录 限定、后置扩展名过滤；
        // 不依赖 dfm-search 库层 “Boolean+fileExtensions” 组合（部分版本返回 0 结果）。
        const QString rawContent = task->content();
        // 类型筛选统一用半角英文冒号 ':'（全角冒号不再作为类型语法）
        const bool hasTypeFilter = rawContent.contains(QLatin1Char(':'));
        if (hasTypeFilter) {
            // 类型过滤搜索：只激活文件搜索项，保持原始文本（FileSearchUtils 再解析）
            searcherData << GRANDSEARCH_CLASS_FILE_DEEPIN;
            qCDebug(logDaemon) << "Type-filter search detected - local FileSearchUtils parsing (raw content kept):"
                               << rawContent;
        } else {
            // 类目、后缀搜索的判读与准备（无冒号的组名/普通关键字搜索）
            QStringList groupList, suffixList, keywordList;
            if (searchHelper->parseKeyword(rawContent, groupList, suffixList, keywordList)) {
                qCDebug(logDaemon) << "Keyword parsing successful - Groups:" << groupList
                                   << "Suffixes:" << suffixList << "Keywords:" << keywordList;
                if (!keywordList.isEmpty() || !suffixList.isEmpty() || !groupList.isEmpty()) {
                    searcherData = checkSearcher(groupList, suffixList, keywordList);
                    const auto &keyword = buildKeywordInJson(groupList, suffixList, keywordList);
                    if (!keyword.isEmpty())
                        task->setContent(keyword);
                }
            }
        }
    }

    for (auto searcher : searchers) {
        Q_ASSERT(searcher);
        const QString name = searcher->name();

        // 判断搜索项是否可用
        if (blankList.contains(name) || !config->value(name, true)
            || (!searcherData.isEmpty() && !searcherData.contains(name)))
            continue;

        // 判断是否激活，若未激活则先激活
        qCDebug(logDaemon) << "Activating searcher:" << name;
        if (searcher->isActive() || searcher->activate()) {
            if (auto worker = searcher->createWorker()) {
                task->join(worker);
                qCDebug(logDaemon) << "Worker created successfully for searcher:" << name;
            } else {
                qCWarning(logDaemon) << "Failed to create worker for searcher:" << name;
            }
        } else {
            qCWarning(logDaemon) << "Failed to activate searcher:" << name;
        }
    }
}

void MainControllerPrivate::buildWorker(TaskCommander *task, const QList<QString> &searchers)
{
    Q_ASSERT(task);
    Q_ASSERT(m_searchers);

    for (const QString &name : searchers) {
        auto searcher = m_searchers->searcher(name);
        if (!searcher) {
            qCWarning(logDaemon) << "Searcher not found:" << name;
            continue;
        }

        qCDebug(logDaemon) << "Activating specific searcher:" << name;
        if (searcher->isActive() || searcher->activate()) {
            if (auto worker = searcher->createWorker()) {
                task->join(worker);
                qCDebug(logDaemon) << "Worker created successfully for specific searcher:" << name;
            } else {
                qCWarning(logDaemon) << "Failed to create worker for specific searcher:" << name;
            }
        } else {
            qCWarning(logDaemon) << "Failed to activate specific searcher:" << name;
        }
    }
}

QStringList MainControllerPrivate::checkSearcher(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList)
{
    qCDebug(logDaemon) << "Checking searcher requirements - Groups:" << groupList
                       << "Suffixes:" << suffixList << "Keywords:" << keywordList;

    QStringList data;
    for (const auto &group : groupList) {
        const auto &searchers = searchHelper->getSearcherByGroupName(group);
        if (searchers.isEmpty()) {
            qCDebug(logDaemon) << "No searchers found for group:" << group;
            continue;
        }
        data.append(searchers);
    }

    // 后缀不为空，说明需要文件搜索项
    if (!suffixList.isEmpty()) {
        data.append(GRANDSEARCH_CLASS_FILE_DEEPIN);
        qCDebug(logDaemon) << "Added file searcher due to suffix requirements";
    } else {
        // 后缀、类目为空，搜索关键字不为空
        // 说明分隔符前后的字段既不是后缀也不是类目
        if (groupList.isEmpty() && !keywordList.isEmpty()) {
            data.append(GRANDSEARCH_CLASS_FILE_DEEPIN);
            data.append(GRANDSEARCH_CLASS_FILE_FULLTEXT);
            data.append(GRANDSEARCH_CLASS_APP_DESKTOP);
            data.append(GRANDSEARCH_CLASS_OCR_TEXT);
            qCDebug(logDaemon) << "Added default searchers (file, full-text and app) for general keyword search";
        }
    }

    qCDebug(logDaemon) << "Searcher check completed - Required searchers:" << data;
    return data;
}

QString MainControllerPrivate::buildKeywordInJson(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList)
{
    qCDebug(logDaemon) << "Building JSON keyword structure";

    QJsonDocument doc;
    QJsonObject obj;
    QJsonArray groupArr, suffixArr, keywordArr;
    for (const auto &group : groupList)
        groupArr.append(group);

    for (const auto &suffix : suffixList)
        suffixArr.append(suffix);

    for (const auto &keyword : keywordList) {
        if (keyword.isEmpty())
            continue;
        keywordArr.append(keyword);
    }

    obj[JSON_GROUP_ATTRIBUTE] = groupArr;
    obj[JSON_SUFFIX_ATTRIBUTE] = suffixArr;
    obj[JSON_KEYWORD_ATTRIBUTE] = keywordArr;
    doc.setObject(obj);
    return doc.toJson(QJsonDocument::Compact);
}

void MainControllerPrivate::dormancy()
{
    qCDebug(logDaemon) << "Dormancy timer triggered - Current task:" << m_currentTask;

    if (m_currentTask && !m_currentTask->isFinished()) {
        qCInfo(logDaemon) << "Task still running, restarting dormancy timer - Task ID:" << m_currentTask->taskID();
        m_dormancy.start();
        return;
    }

    qCInfo(logDaemon) << "Entering dormancy mode - all tasks completed";
    if (m_searchers) {
        m_searchers->dormancy();
        qCDebug(logDaemon) << "Searcher group dormancy activated";
    } else {
        qCWarning(logDaemon) << "Cannot activate dormancy - searcher group not initialized";
    }
}

MainController::MainController(QObject *parent)
    : QObject(parent),
      d(new MainControllerPrivate(this))
{
    qCDebug(logDaemon) << "MainController constructor completed";
}

bool MainController::init()
{
    qCInfo(logDaemon) << "MainController initialization started";

    Q_ASSERT(d->m_searchers == nullptr);

    d->m_searchers = new SearcherGroup(this);
    if (!d->m_searchers->init()) {
        qCCritical(logDaemon) << "Failed to initialize SearcherGroup";
        return false;
    }

    // 初始化配置模块
    return ConfigerIns->init();
}

bool MainController::newSearch(const QString &key)
{
    qCInfo(logDaemon) << "New search request received - Key length:" << key.size()
                      << "Current task exists:" << (d->m_currentTask != nullptr);

    if (Q_UNLIKELY(key.isEmpty())) {
        qCWarning(logDaemon) << "Search key is empty, ignoring request";
        return false;
    }

    auto func = [this](TaskCommander *task) {
        d->buildWorker(task);
    };

    return d->buildTask(key, func);
}

void MainController::terminate()
{
    qCDebug(logDaemon) << "Termination requested";

    // 停止任务
    if (d->m_currentTask) {
        qCDebug(logDaemon) << "Terminating current task - ID:" << d->m_currentTask->taskID();
        disconnect(d->m_currentTask, nullptr, this, nullptr);
        d->m_currentTask->stop();
        d->m_currentTask->deleteSelf();
        d->m_currentTask = nullptr;
        qCDebug(logDaemon) << "Current task terminated successfully";
    } else {
        qCDebug(logDaemon) << "No current task to terminate";
    }
}

QByteArray MainController::getResults() const
{
    qCDebug(logDaemon) << "Results requested";

    if (d->m_currentTask) {
        MatchedItemMap items = d->m_currentTask->getResults();
        qCDebug(logDaemon) << "Retrieved results from current task - Item groups:" << items.size();

        // 序列化
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << items;

        return bytes;
    }

    qCDebug(logDaemon) << "No current task - returning empty results";
    return QByteArray();
}

QByteArray MainController::readBuffer() const
{
    qCDebug(logDaemon) << "Buffer read requested";

    if (d->m_currentTask) {
        MatchedItemMap items = d->m_currentTask->readBuffer();
        qCDebug(logDaemon) << "Retrieved buffer from current task - Item groups:" << items.size();

        // 序列化
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << items;

        return bytes;
    }

    qCDebug(logDaemon) << "No current task - returning empty buffer";
    return QByteArray();
}

bool MainController::isEmptyBuffer() const
{
    if (d->m_currentTask) {
        return d->m_currentTask->isEmptyBuffer();
    }

    qCDebug(logDaemon) << "No current task - buffer considered empty";
    return true;
}

bool MainController::searcherAction(const QString &name, const QString &action, const QString &item)
{
    qCDebug(logDaemon) << "Searcher action requested - Searcher:" << name
                       << "Action:" << action << "Item:" << item;

    if (name.isEmpty() || action.isEmpty() || item.isEmpty()) {
        qCWarning(logDaemon) << "Invalid searcher action parameters - Name:" << name
                             << "Action:" << action << "Item:" << item;
        return false;
    }

    if (auto searcher = d->m_searchers->searcher(name)) {
        return searcher->action(action, item);
    }

    qCWarning(logDaemon) << "Searcher not found for action:" << name;
    return false;
}
