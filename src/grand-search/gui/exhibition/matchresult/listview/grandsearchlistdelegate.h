/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu <houchengqiu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRANDSEARCHLISTDELEGATE_H
#define GRANDSEARCHLISTDELEGATE_H

#include <DListView>
#include <DHiDPIHelper>

#include <QScopedPointer>
#include <DStyledItemDelegate>

class GrandSearchListDelegate : public Dtk::Widget::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GrandSearchListDelegate(QAbstractItemView *parent = Q_NULLPTR);
    ~GrandSearchListDelegate() override;

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void drawSelectState(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawSearchResultText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawTailText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, int textMaxWidth, int actualStartX) const;
    void drawTailDetailedInfo(QPainter *painter, const QStyleOptionViewItem &option, const QString &text, const QColor &color, const QFont &font, const QFontMetrics &fontMetrics, int &startX) const;

    void calcTailShowInfo(const int totalTailWidth, int &tailCount, int &averageWidth) const;
    QMap<int, QString> calcTailShowData(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
    QMap<int, QString> calcTailShowDataByMaxWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
    QMap<int, QString> calcTailShowDataByOptimalWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
};

#endif // GRANDSEARCHLISTDELEGATE_H
