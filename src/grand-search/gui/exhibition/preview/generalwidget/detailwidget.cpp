/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "detailwidget.h"
#include "detailitem.h"

#include <QLocale>

#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

DetailWidget::DetailWidget(QWidget *parent)
    : DWidget(parent)
{
    m_detailItems.clear();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(1);

    this->setLayout(m_mainLayout);

    // 暂不考虑软件使用过程中语言切换，只获取一次以提升效率
    QString systemName = QLocale::system().name().simplified();

    // 中文环境下，标签与内容分别对齐
    if (systemName.startsWith("zh"))
        m_alignment = true;
}

void DetailWidget::setDetailInfoList(const GrandSearch::DetailInfoList &list)
{
    qDeleteAll(m_detailItems);
    m_detailItems.clear();

    int maxTagWidth = 0;
    for (int i = 0; i < list.count(); i++) {

        auto item = new DetailItem(this);

        if (0 == i)                     // 第一行绘制上圆角
            item->setTopRound(true);
        if (list.count() - 1 == i)      // 最后一行绘制下圆角
            item->setBottomRound(true);

        item->setDetailInfo(list.at(i));

        // 获取最大标签宽度
        maxTagWidth = item->tagWidth() > maxTagWidth ? item->tagWidth() : maxTagWidth;

        m_detailItems.append(item);
        m_mainLayout->addWidget(item);
    }

    // 标签与内容分别对齐
    if (m_alignment) {
        for (auto item : m_detailItems) {
            // 固定标签宽度，内容宽度通过内部布局器自动计算
            item->setTagWidth(maxTagWidth);
        }
    }

}
