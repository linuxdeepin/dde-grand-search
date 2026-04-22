// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include "thumbnailgenerator.h"

#include <QList>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QString>

namespace GrandSearch {

// 前向声明
enum class ThumbnailSize;

/**
 * @brief 缩略图提供者
 *
 * 单例模式，作为缩略图系统的核心协调者，负责：
 * - 管理所有注册的缩略图生成器
 * - 根据 mimetype 分发到合适的生成器
 * - 协调缓存和任务管理器
 * - 提供统一的缩略图请求接口
 */
class ThumbnailProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return ThumbnailProvider 实例指针
     */
    static ThumbnailProvider *instance();

    /**
     * @brief 注册缩略图生成器
     * @param generator 生成器实例（Provider 不拥有所有权）
     */
    void registerGenerator(ThumbnailGenerator *generator);

    /**
     * @brief 注销缩略图生成器
     * @param generator 生成器实例
     */
    void unregisterGenerator(ThumbnailGenerator *generator);

    /**
     * @brief 请求缩略图（异步）
     * @param filePath 文件路径
     * @param mimetype MIME 类型
     * @param size 目标尺寸
     * @param priority 优先级
     *
     * 此方法会先检查缓存，如果缓存存在则直接发送 thumbnailReady 信号。
     * 否则提交异步任务生成缩略图。
     */
    void requestThumbnail(const QString &filePath, const QString &mimetype,
                          const ThumbnailSize &size, int priority = 0);

    /**
     * @brief 同步获取缩略图（阻塞）
     * @param filePath 文件路径
     * @param mimetype MIME 类型
     * @param size 目标尺寸
     * @return 缩略图，如果生成失败返回空 QPixmap
     *
     * 注意：此方法会阻塞当前线程，建议仅在后台线程中使用。
     */
    QPixmap getThumbnailSync(const QString &filePath, const QString &mimetype,
                             const ThumbnailSize &size);

    /**
     * @brief 取消指定文件的缩略图请求
     * @param filePath 文件路径
     */
    void cancelRequest(const QString &filePath);

    /**
     * @brief 检查是否支持指定 mimetype
     * @param mimetype MIME 类型
     * @return 如果有生成器支持返回 true，否则返回 false
     */
    bool isSupported(const QString &mimetype) const;

    /**
     * @brief 获取所有支持的 mimetype 列表
     * @return 支持的 mimetype 列表
     */
    QStringList supportedMimeTypes() const;

signals:
    /**
     * @brief 缩略图生成完成信号
     * @param filePath 文件路径
     * @param thumbnail 生成的缩略图
     */
    void thumbnailReady(const QString &filePath, const QPixmap &thumbnail);

    /**
     * @brief 缩略图生成失败信号
     * @param filePath 文件路径
     */
    void thumbnailFailed(const QString &filePath);

private slots:
    /**
     * @brief 处理任务管理器的完成信号
     */
    void onThumbnailReady(const QString &filePath, const QPixmap &thumbnail);
    void onThumbnailFailed(const QString &filePath);

private:
    explicit ThumbnailProvider(QObject *parent = nullptr);
    ~ThumbnailProvider() override;

    // 禁止拷贝
    ThumbnailProvider(const ThumbnailProvider &) = delete;
    ThumbnailProvider &operator=(const ThumbnailProvider &) = delete;

    /**
     * @brief 查找合适的生成器
     * @param mimetype MIME 类型
     * @return 生成器指针，如果没有找到返回 nullptr
     */
    ThumbnailGenerator *findGenerator(const QString &mimetype) const;

private:
    static ThumbnailProvider *s_instance;

    QList<ThumbnailGenerator *> m_generators;   // 已注册的生成器列表
    mutable QMutex m_mutex;   // 线程安全锁
};

}   // namespace GrandSearch

#endif   // THUMBNAILPROVIDER_H
