// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHLISTVIEW_H
#define GRANDSEARCHLISTVIEW_H

#include "global/matcheditem.h"
#include "gui/datadefine.h"

#include <DListView>

#include <QPixmap>

#define THUMBNAIL_ROLE Qt::UserRole + 1
#define DATA_ROLE Qt::UserRole + 2

namespace GrandSearch {

class GrandSearchListModel;
class GrandSearchListDelegate;
class GrandSearchListView : public Dtk::Widget::DListView
{
    Q_OBJECT
public:
    explicit GrandSearchListView(QWidget *parent = Q_NULLPTR);
    ~GrandSearchListView() override;

    void setMatchedItems(const MatchedItems &items);
    void addRow(const MatchedItem &item);
    void addRow(const MatchedItem &item, const int level);
    void addRows(const MatchedItems &items);
    void addRows(const MatchedItems &items, const int level);
    int insertRow(int nRow, const MatchedItem &item);
    void insertRows(int nRow, const MatchedItems &items);
    // 从给定行开始删除指定行数
    void removeRows(int nRow, int nCount);

    // 对应level级别项的数量,默认返回所有层级项的总和
    int levelItemCount(const int level = -1);

    int rowCount();
    int getThemeType() const;

    void clear();

    void updatePreviewItemState(const bool preview);
    bool isPreviewItem() const;

    /**
     * @brief 设置当前搜索关键词并清理旧的高亮任务
     * @param keyword 新的搜索关键词
     */
    void setSearchKeyword(const QString &keyword);

public slots:
    void onSetThemeType(int type);

    /**
     * @brief 处理高亮内容获取完成（由 MatchWidget 统一连接信号后路由调用）
     * @param keyword 搜索关键词（taskId）
     * @param filePath 文件路径
     * @param content 高亮内容
     */
    void onHighlightReady(const QString &keyword, const QString &filePath, const QString &content);

signals:
    void sigCurrentItemChanged(const MatchedItem &item);
    void sigItemClicked();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:
    /**
     * @brief 处理缩略图生成完成
     * @param filePath 文件路径
     * @param thumbnail 生成的缩略图
     */
    void onThumbnailReady(const QString &filePath, const QPixmap &thumbnail);

private:
    QString cacheDir();
    void setData(const QModelIndex &index, const MatchedItem &item);
    int levelItemLastRow(const int level);

    /**
     * @brief 更新指定文件路径对应的项的缩略图
     * @param filePath 文件路径
     * @param thumbnail 缩略图
     */
    void updateThumbnail(const QString &filePath, const QPixmap &thumbnail);

    /**
     * @brief 更新指定文件路径对应的项的高亮内容
     * @param filePath 文件路径
     * @param content 高亮内容
     */
    void updateHighlight(const QString &filePath, const QString &content);

    /**
     * @brief 请求指定项的高亮内容（如果尚未请求）
     * @param item 搜索结果项
     * @param highPriority 是否为高优先级（可见区域）
     */
    void requestHighlightContent(const MatchedItem &item, bool highPriority = false);

private:
    GrandSearchListModel *m_model = nullptr;
    GrandSearchListDelegate *m_delegate = nullptr;

    int m_themeType = 1;   // 当前应用主题类型 1:浅色 2:深色 默认1:浅色
    MatchedItems m_matchedItems;

    bool m_isPreviewItem = false;
    QString m_currentKeyword;  // 当前搜索关键词，用于高亮任务管理
};

}

#endif   // GRANDSEARCHLISTVIEW_H
