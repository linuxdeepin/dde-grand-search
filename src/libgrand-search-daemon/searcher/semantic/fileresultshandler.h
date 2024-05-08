// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILERESULTSHANDLER_H
#define FILERESULTSHANDLER_H

#include "searcher/file/filesearchutils.h"

#include <QSet>

namespace GrandSearch {

class FileResultsHandler
{
public:
    explicit FileResultsHandler();
    bool appendTo(const QString &file, MatchedItemMap &container);
    bool isResultLimit() const;
    inline int itemWeight(const QString &file) const{
        QReadLocker lk(&m_lock);
        return m_resultWeight.value(file, 0);
    }

    inline void setItemWeight(const QString &file, int w) {
        QWriteLocker lk(&m_lock);
        m_resultWeight[file] = w;
    }

    inline QHash<QString, int> allItemWeight() const {
        QReadLocker lk(&m_lock);
        return m_resultWeight;
    }

    inline int resultCount() const {
        QReadLocker lk(&m_lock);
        return m_tmpSearchResults.size();
    }
private:
    void initConfig();
protected:
    mutable QReadWriteLock m_lock;
    QSet<QString> m_tmpSearchResults;     // 存储所有的搜索结果，用于去重
    QHash<QString, int> m_resultWeight;
    QHash<FileSearchUtils::Group, quint32> m_resultCountHash; // 记录各类型文件搜索结果数量
};

}

#endif // FILERESULTSHANDLER_H
