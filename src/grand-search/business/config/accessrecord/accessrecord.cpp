// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessrecord.h"
#include "global/searchconfigdefine.h"
#include "utils/utils.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>
#include <QString>
#include <QApplication>
#include <QtConcurrent>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

class AccessRecordGlobal : public AccessRecord
{
};
Q_GLOBAL_STATIC(AccessRecordGlobal, accessRecordGlobal)

AccessRecord *AccessRecord::instance()
{
    return accessRecordGlobal;
}

void AccessRecord::startParseRecord()
{
    qCDebug(logGrandSearch) << "Starting access record parsing";

    std::call_once(m_initFlag, []() {
        qCDebug(logGrandSearch) << "Initializing access record parsing - Path:" << AccessRecord::instance()->m_recordPath;
        // 异步解析
        QtConcurrent::run(&AccessRecord::parseRecord, AccessRecord::instance()->m_recordPath);
    });
}

/**
 * @brief AccessRecord::getRecord 主线程获取数据
 */
QHash<QString, QHash<QString, int>> AccessRecord::getRecord()
{
    return m_recordHash;
}

/**
 * @brief AccessRecord::updateRecord 根据用户的点击更新记录
 * @param matchedItem 用户点击的类目
 * @param time 用户点击的时间
 */
void AccessRecord::updateRecord(const MatchedItem &matchedItem, qint64 time)
{
    const QString &seacher = matchedItem.searcher;
    const QString &item = matchedItem.item;

    qCDebug(logGrandSearch) << "Updating access record - Searcher:" << seacher
                            << "Item:" << item
                            << "Time:" << time;

    QJsonObject recordObj = m_recordObj;

    if (!recordObj.contains(seacher)) {
        QJsonArray times;
        times.append(time);
        QJsonObject itemObj;
        itemObj.insert(item, times);
        recordObj.insert(seacher, itemObj);
        qCDebug(logGrandSearch) << "Created new searcher entry:" << seacher;
    } else {
        QJsonObject::iterator searcherIt = recordObj.begin();
        while (searcherIt != recordObj.end()) {
            if (searcherIt.key() == seacher) {
                QJsonObject itemsObj = searcherIt.value().toObject();
                if (!itemsObj.contains(item)) {
                    QJsonArray times;
                    times.append(time);
                    itemsObj.insert(item, times);
                    qCDebug(logGrandSearch) << "Created new item entry - Searcher:" << seacher << "Item:" << item;
                } else {
                    QJsonObject::iterator itemIt = itemsObj.begin();
                    while (itemIt != itemsObj.end()) {
                        if (itemIt.key() == item) {
                            QJsonArray times = itemIt.value().toArray();
                            times.append(double(time));
                            itemsObj.insert(item, times);
                            qCDebug(logGrandSearch) << "Updated existing item - Searcher:" << seacher
                                                    << "Item:" << item << "Access count:" << times.size();
                            break;
                        }
                        ++itemIt;
                    }
                }
                recordObj.insert(seacher, itemsObj);
                break;
            }
            ++searcherIt;
        }
    }

    m_recordObj = recordObj;
    qCDebug(logGrandSearch) << "Access record update completed";
}

/**
 * @brief AccessRecord::sync 主线程将更新后的数据回写到文件中
 */
void AccessRecord::sync()
{
    if (!m_finished) {
        qCWarning(logGrandSearch) << "Cannot sync records - Parsing not finished";
        return;
    }

    qCDebug(logGrandSearch) << "Syncing access records to file:" << m_recordPath;
    QFile file(m_recordPath);
    if (!file.open(QFile::WriteOnly)) {
        qCWarning(logGrandSearch) << "Failed to open record file for writing:" << m_recordPath
                                  << "Error:" << file.errorString();
        return;
    }

    QJsonDocument doc(m_recordObj);
    QByteArray data = doc.toJson();
    file.write(data);
    file.close();
    qCDebug(logGrandSearch) << "Successfully synced access records - Size:" << data.size() << "bytes";
}

AccessRecord::AccessRecord()
{
    qCDebug(logGrandSearch) << "Creating AccessRecord instance";

    qRegisterMetaType<QHash<QString, QHash<QString, int>>>();
    auto recordPath = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first();
    m_recordPath = recordPath
            + "/" + QApplication::organizationName()
            + "/" + GRANDSEARCH_NAME
            + "/" + "accessrecord.json";

    qCDebug(logGrandSearch) << "Initializing access record manager - Path:" << m_recordPath;

    connect(this, &AccessRecord::sigParseFinished, this, [this](const QJsonObject &recordObj, const QHash<QString, QHash<QString, int>> &recordHash) {
        qCInfo(logGrandSearch) << "Record parsing completed - Updating data";
        m_recordObj = recordObj;
        m_recordHash = recordHash;
        m_finished = true;
        qCDebug(logGrandSearch) << "Access record initialization completed - Searchers:" << recordHash.size();
    });

    qCDebug(logGrandSearch) << "AccessRecord instance created successfully";
}

AccessRecord::~AccessRecord()
{
    qCDebug(logGrandSearch) << "Destroying AccessRecord instance";
}

/**
 * @brief AccessRecord::ParseRecord 解析记录文件
 */
void AccessRecord::parseRecord(QString recordPath)
{
    qCDebug(logGrandSearch) << "Starting record file parsing - Path:" << recordPath;

    QFileInfo recordFile(recordPath);
    // 创建文件
    if (!recordFile.exists()) {
        qCDebug(logGrandSearch) << "Creating new record file:" << recordPath;
        recordFile.absoluteDir().mkpath(".");
        QFile file(recordPath);
        file.open(QFile::NewOnly);
        QJsonObject version;
        version.insert("version", "1.0");
        QJsonDocument doc(version);
        QByteArray data = doc.toJson();
        file.write(data);
        file.close();
        qCInfo(logGrandSearch) << "Created new record file with version 1.0";
    }
    parseJson(recordPath);
}

/**
 * @brief AccessRecord::parseJson 解析json数据
 * @param recordPath 文件路径
 */
void AccessRecord::parseJson(const QString &recordPath)
{
    qCDebug(logGrandSearch) << "Parsing record file:" << recordPath;
    QFile file(recordPath);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(logGrandSearch) << "Failed to open record file for reading:" << recordPath
                                  << "Error:" << file.errorString();
        return;
    }

    QByteArray allData = file.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument doc(QJsonDocument::fromJson(allData, &jsonError));

    static const int validTime = 7 * 24 * 60 * 60;

    // 记录计算次数的searcher
    QHash<QString, QHash<QString, int>> searcherHash;
    QJsonObject searcherObj;
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(logGrandSearch) << "JSON parsing error:" << jsonError.errorString()
                                  << "Offset:" << jsonError.offset;
        emit AccessRecord::instance()->sigParseFinished(searcherObj, searcherHash, QPrivateSignal());
        return;
    }

    qCDebug(logGrandSearch) << "JSON parsing successful, processing data";

    searcherObj = doc.object();
    for (auto searcherIt = searcherObj.begin(); searcherIt != searcherObj.end();) {
        if (searcherIt.key() == "version") {
            ++searcherIt;
            continue;
        }

        QJsonObject itemsObj = searcherIt.value().toObject();
        // 记录计算次数的item
        QHash<QString, int> itemsHash;
        for (auto itemIt = itemsObj.begin(); itemIt != itemsObj.end();) {
            QJsonArray timeArray = itemIt.value().toArray();
            for (auto timeIt = timeArray.begin(); timeIt != timeArray.end();) {
                qint64 timeT = (*timeIt).toDouble();
                // 获取当前时间戳
                const qint64 currentTimeT = QDateTime::currentDateTime().toSecsSinceEpoch();
                // 计算两个时间戳的差
                const qint64 intervalTime = currentTimeT - timeT;
                // 时间戳的差大于七天
                if (intervalTime > validTime || intervalTime < 0) {
                    timeIt = timeArray.erase(timeIt);
                } else {
                    ++timeIt;
                }
            }

            if (timeArray.isEmpty()) {
                // 时间列表为空，删除该item对象
                itemIt = itemsObj.erase(itemIt);
            } else {
                auto newIt = itemsObj.insert(itemIt.key(), timeArray);
                Q_ASSERT(newIt == itemIt);
                ++itemIt;
            }
        }

        if (itemsObj.empty()) {
            // item对象为空，删除该searcher对象
            searcherIt = searcherObj.erase(searcherIt);
        } else {
            for (auto tempItemIt = itemsObj.begin(); tempItemIt != itemsObj.end();) {
                QJsonArray timeArray = tempItemIt.value().toArray();
                int times = timeArray.size();
                if (times > 20) {
                    itemsHash.insert(tempItemIt.key(), 20);
                } else {
                    itemsHash.insert(tempItemIt.key(), times);
                }
                ++tempItemIt;
            }
            searcherHash.insert(searcherIt.key(), itemsHash);
            auto newIt = searcherObj.insert(searcherIt.key(), itemsObj);
            Q_ASSERT(newIt == searcherIt);
            ++searcherIt;
        }
    }

    qCDebug(logGrandSearch) << "Record parsing completed - Searchers:" << searcherHash.size();
    emit AccessRecord::instance()->sigParseFinished(searcherObj, searcherHash, QPrivateSignal());
}
