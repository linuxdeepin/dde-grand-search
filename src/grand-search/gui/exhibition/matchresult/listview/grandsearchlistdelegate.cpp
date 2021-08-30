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
#include "grandsearchlistdelegate.h"
#include "grandsearchlistview.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QStyleOptionViewItem>
#include <QTextCursor>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QToolTip>

#define ListItemSpace             10       // 列表图标与文本间间距
#define ListItemHeight            36       // 列表行高
#define ListIconSize              24       // 列表图标大小
#define ListRowWidth              350      // 列表行宽
#define ListIconMargin            6        // 列表图标边距距

DWIDGET_USE_NAMESPACE

GrandSearchListDelegate::GrandSearchListDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

GrandSearchListDelegate::~GrandSearchListDelegate()
{

}

void GrandSearchListDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    DStyledItemDelegate::initStyleOption(option, index);
}

bool GrandSearchListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return true;
}

void GrandSearchListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    // 绘制选中状态(UI设计图暂无选择外框线）
    drawSelectState(painter, option, index);

    // 绘制匹配结果文本
    drawSearchResultText(painter, option, index);
}

QSize GrandSearchListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(ListRowWidth, ListItemHeight);
}

static void hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

bool GrandSearchListDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(Qt::ToolTipRole).toString();

        if (tooltip.isEmpty()) {
            hideTooltipImmediately();
        } else {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
            QToolTip::showText(event->globalPos(), strtooltip, view);
        }

        return true;
    }

    return DStyledItemDelegate::helpEvent(event, view, option, index);
}

void GrandSearchListDelegate::drawSelectState(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    if (option.state & QStyle::State_MouseOver) {
        painter->save();
        painter->setPen(Qt::NoPen);
        QColor hovertColor(option.palette.highlight().color());

        if (option.state & QStyle::State_Selected)
            hovertColor.setAlphaF(0.2);

        painter->setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(0, 0, 0, 0);
        painter->drawRoundedRect(selecteColorRect, 5, 5);
        painter->restore();
    }

}

void GrandSearchListDelegate::drawSearchResultText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor textColor;
    QColor lightColor;

    const GrandSearchListView *listview = qobject_cast<const GrandSearchListView *>(option.widget);

    // 主题改变需要修改color
    if (listview->getThemeType() == 2) {
        textColor = QColor("#FFFFFF");
    } else {
        textColor = QColor("#000000");
    }

    DPalette pa = option.palette;
    QBrush selectBrush = pa.brush(QPalette::Active, DPalette:: Highlight);
    QColor selectColor = selectBrush.color();
    lightColor = selectColor;

    if ((option.state & QStyle::State_MouseOver)
            || (index.isValid() && index == listview->currentIndex())) {
        textColor = QColor("#FFFFFF");
        lightColor = QColor("#FFFFFF");
    }

    // 设置字体
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    QString mtext;
    mtext = index.data(DATA_ROLE).value<MatchedItem>().name;

    int listItemTextMaxWidth = option.rect.width() / 2 - ListItemSpace * 2 - ListIconSize;
    //预览界面正在显示，结果列表截断显示宽度改为列表显示最大宽度
    if (listItemTextMaxWidth < ListRowWidth / 2)
        listItemTextMaxWidth = option.rect.width() - ListItemSpace * 2 -ListIconSize;
    QFontMetricsF fontWidth(fontT6);
    mtext = fontWidth.elidedText(mtext, Qt::ElideRight, listItemTextMaxWidth);
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
    QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
    viewOption.text = "";
    pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

    // 设置文字边距，保证绘制文字居中
    QTextDocument document;
    document.setDocumentMargin(0);
    document.setPlainText(mtext);
    QTextCursor cursor(&document);

    cursor.beginEditBlock();
    QTextCursor testcursor(&document);
    testcursor.select(QTextCursor::LineUnderCursor);
    QTextCharFormat fmt;
    fmt.setForeground(textColor);
    fmt.setFont(fontT6);
    testcursor.mergeCharFormat(fmt);
    testcursor.clearSelection();
    testcursor.movePosition(QTextCursor::EndOfLine);
    cursor.endEditBlock();

    // 动态计算边距，保证调整字体大小时绘制居中
    QAbstractTextDocumentLayout::PaintContext paintContext;
    int margin = static_cast<int>(((option.rect.height() - fontWidth.height()) / 2));
    QRect textRect(ListItemSpace * 2 + ListIconSize, option.rect.y() + margin, listItemTextMaxWidth, option.rect.height());
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    document.documentLayout()->draw(painter, paintContext);
    painter->restore();
}

