/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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
#include "blacklistdelegate.h"
#include "blacklistview.h"
#include "blacklistmodel.h"

#include <DGuiApplicationHelper>
#include <DPalette>
#include <DPaletteHelper>

#include <QColor>
#include <QPainter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>

#define ListItemHeight      36      // 列表行高
#define ListItemWidth       456     // 列表行宽
#define ListIconSize        24      // 列表图标大小
#define ListItemSpace       10      // 列表图标与文本间间距
#define ListIconMargin      10      // 列表图标边距
#define ListTextMaxWidth    362     // 列表文本最长显示宽度

BlackListDelegate::BlackListDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{

}

BlackListDelegate::~BlackListDelegate()
{

}

QSize BlackListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(ListItemWidth, ListItemHeight);
}

void BlackListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->setRenderHint(QPainter::Antialiasing);
    drawPathsText(painter, option, index);
    drawItemBackground(painter, option, index);
}

void BlackListDelegate::drawPathsText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
      QFont pathFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);

      const QString &path = index.data(DATAROLE).value<QString>();
      QFontMetrics pathFontMetrics(pathFont);
      QString elidedPath = pathFontMetrics.elidedText(path, Qt::ElideMiddle, ListTextMaxWidth);

      const BlackListView *listView = qobject_cast<const BlackListView *>(option.widget);
      auto selected = listView->selectionModel();
      QColor pathColor;
      if (selected->isSelected(index)) {
          pathColor = QColor("#FFFFFF");
      } else {
          if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
              pathColor = QColor("#414D68");
          } else {
              pathColor = QColor("#C0C6D4");
          }
      }

      QTextDocument pathDocument;
      pathDocument.setDocumentMargin(0);
      pathDocument.setPlainText(elidedPath);
      QTextCursor pathCursor(&pathDocument);

      pathCursor.beginEditBlock();
      pathCursor.select(QTextCursor::LineUnderCursor);
      QTextCharFormat pathFmt;
      pathFmt.setFont(pathFont);
      pathFmt.setForeground(pathColor);
      pathCursor.mergeCharFormat(pathFmt);
      pathCursor.clearSelection();
      pathCursor.endEditBlock();

      QAbstractTextDocumentLayout::PaintContext paintContext;
      int pathMargin = static_cast<int>((option.rect.height() - pathFontMetrics.height()) / 2);
      int actualPathWidth = pathFontMetrics.size(Qt::TextSingleLine, elidedPath).width();
      int actualStartX = ListIconSize + ListItemSpace + ListIconMargin;
      QRect pathTextRect(actualStartX, option.rect.y() + pathMargin, actualPathWidth, option.rect.height());

      painter->save();
      painter->translate(pathTextRect.topLeft());
      painter->setClipRect(pathTextRect.translated(-pathTextRect.topLeft()));
      pathDocument.documentLayout()->draw(painter, paintContext);
      painter->restore();
}

void BlackListDelegate::drawItemBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() % 2 == 1)
        return;
    painter->save();
    if (option.widget) {
        DPalette pl(DPaletteHelper::instance()->palette(option.widget));
        painter->setPen(Qt::NoPen);
        painter->setBrush(pl.brush(DPalette::ItemBackground));
    }
    QRect rect = option.rect.adjusted(0, 0, -1, -1);
    painter->drawRoundedRect(rect, 8, 8);
    painter->restore();
}
