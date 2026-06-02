// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHLISTDELEGATE_H
#define GRANDSEARCHLISTDELEGATE_H

#include <DListView>
#include <DHiDPIHelper>

#include <QScopedPointer>
#include <DStyledItemDelegate>

namespace GrandSearch {

// tooltip 省略区域：记录绘制时被省略的文本区域和完整内容
struct TooltipRegion
{
    QRect rect;   // 该区域在 item 中的绘制矩形
    QString fullText;   // 被省略前的完整文本
};

class GrandSearchListDelegate : public Dtk::Widget::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GrandSearchListDelegate(QAbstractItemView *parent = Q_NULLPTR);
    ~GrandSearchListDelegate() override;

    // 清除 tooltip 区域缓存（在列表清空时调用）
    void clearTooltipCache();

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void drawSelectState(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    // 在绘制时记录省略的 tooltip 区域
    void recordTooltipRegion(int row, const QRect &rect, const QString &fullText) const;

    mutable QHash<int, QList<TooltipRegion>> m_tooltipRegionCache;
    void drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawSearchResultText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawItemName(QPainter *painter, const QModelIndex &index, const QStyleOptionViewItem &option,
                      const QString &name, const QString &searcher, const QStringList &keywords,
                      const QFont &font, const QFontMetrics &fontMetrics, const QColor &textColor,
                      bool isDark, int startY) const;
    void drawMatchedContext(QPainter *painter, const QModelIndex &index, const QStyleOptionViewItem &option,
                            const QString &matchedContext, const QStringList &keywords,
                            const QFont &font, const QFontMetrics &fontMetrics,
                            bool isDark, int startY, int startX = 0) const;
    int drawTailText(QPainter *painter, const QModelIndex &index, const QStringList &tailList,
                     const QFont &font, const QFontMetrics &fontMetrics, const QColor &color,
                     int maxWidth, int startX, int startY) const;
    void drawSecondLineText(QPainter *painter, const QString &text, const QFont &font,
                            const QFontMetrics &fontMetrics, const QColor &color,
                            int &startX, int startY) const;

    void calcTailShowInfo(const int totalTailWidth, int &tailCount, int &averageWidth) const;
    QMap<int, QString> calcTailShowData(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
    QMap<int, QString> calcTailShowDataByMaxWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
    QMap<int, QString> calcTailShowDataByOptimalWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
};

}

#endif   // GRANDSEARCHLISTDELEGATE_H
