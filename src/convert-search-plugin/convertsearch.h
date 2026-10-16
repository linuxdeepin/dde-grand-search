// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONVERTSEARCH_H
#define CONVERTSEARCH_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QMutex>
#include <QJsonObject>

#include "providers/currencyprovider.h"

// 全局搜索转换插件主体：实现 Search / Stop / Action 三个 DBus 方法。
class ConvertSearch : public QObject {
    Q_OBJECT
public:
    explicit ConvertSearch(QObject *parent = nullptr);
    ~ConvertSearch() override;

    QString search(const QString &json);
    bool stop(const QString &json);
    bool action(const QString &json);

private:
    struct SearchResultCache {
        QJsonObject result; // 完整结果 JSON（用于 Action 时定位）
    };

    // 解析 daemon 入参
    static bool parseInput(const QString &json, QString &mID, QString &cont);

    CurrencyProvider *m_currency;        // 常驻，负责汇率缓存与联网
    QHash<QString, QJsonObject> m_lastResults;
    QHash<QString, QString> m_lastCont;   // mID → 原始查询文本（UOS AI 联动用）
    QMutex m_mutex;
};

#endif // CONVERTSEARCH_H
