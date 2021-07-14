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
#include <QStyledItemDelegate>

class GrandSearchListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GrandSearchListDelegate(QWidget *parent = Q_NULLPTR);
    ~GrandSearchListDelegate() override;

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void drawSelectState(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawUpDownSelected(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawSearchResultIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawSearchResultText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // GRANDSEARCHLISTDELEGATE_H
