// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXSTATUSMONITOR_H
#define INDEXSTATUSMONITOR_H

#include <QObject>
#include <QPointer>
#include <QHash>

class QDBusInterface;
class QDBusPendingCallWatcher;

namespace GrandSearch {

/**
 * @brief 文管索引状态监听器
 *
 * 通过文管提供的 D-Bus 接口监听全文索引（TextIndex）与图片 OCR 索引
 * （OcrIndex）的状态：GetIndexStatus() 查询、IndexStatusChanged 信号实时
 * 通知、ForceUpdateIndex() 触发重试/继续更新。服务未启动时自动拉起，
 * 调用失败静默降级（发出 success=false 的结果信号）。
 */
class IndexStatusMonitor : public QObject
{
    Q_OBJECT

public:
    // 索引类型：全文索引 / OCR 索引
    enum class Index { Text, Ocr };

    explicit IndexStatusMonitor(QObject *parent = nullptr);
    ~IndexStatusMonitor() override;

    // 异步查询两路索引的当前状态，结果经 indexStatusResult 信号返回
    void getIndexStatus();

    // 请求指定索引强制更新索引
    // manual=true 强制重试（Failed 后重试）；manual=false 绕过电池/省电限制继续更新
    void forceUpdateIndex(Index idx, const QStringList &paths, bool manual);

signals:
    void indexStatusResult(Index idx, const QString &state, const QString &grade, bool success);
    void indexStatusChanged(Index idx, const QString &state, const QString &grade);

private slots:
    void onTextStatusChanged(const QString &state, const QString &grade);
    void onOcrStatusChanged(const QString &state, const QString &grade);

private:
    bool ensureInterface(Index idx);
    QDBusInterface *interface(Index idx);
    void queryIndexStatus(Index idx);
    void handleStatusReply(Index idx, QDBusPendingCallWatcher *watcher);

    static QString serviceName(Index idx);
    static QString objectPath(Index idx);

private:
    QHash<int, QPointer<QDBusInterface>> m_interfaces;
};

}

#endif   // INDEXSTATUSMONITOR_H
