/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#include "exhibitionwidget_p.h"
#include "exhibitionwidget.h"
#include "matchresult/matchwidget.h"
#include "preview/previewwidget.h"
#include "utils/utils.h"

#include <DFrame>

#include <QPalette>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define MARGIN_SIZE 10

ExhibitionWidgetPrivate::ExhibitionWidgetPrivate(ExhibitionWidget *parent)
    : q_p(parent)
{

}

ExhibitionWidget::ExhibitionWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new ExhibitionWidgetPrivate(this))
{
    initUi();
    initConnect();
}

ExhibitionWidget::~ExhibitionWidget()
{

}

void ExhibitionWidget::clearData()
{
    Q_ASSERT(m_matchWidget);

    m_matchWidget->clearMatchedData();
    m_previewWidget->hide();
}

void ExhibitionWidget::onSelectNextItem()
{
    if (this->isHidden())
        return;

    Q_ASSERT(m_matchWidget);

    m_matchWidget->selectNextItem();
}

void ExhibitionWidget::onSelectPreviousItem()
{
    if (this->isHidden())
        return;

    Q_ASSERT(m_matchWidget);

    m_matchWidget->selectPreviousItem();
}

void ExhibitionWidget::onHandleItem()
{
    if (this->isHidden())
        return;

    Q_ASSERT(m_matchWidget);

    m_matchWidget->handleItem();
}

void ExhibitionWidget::appendMatchedData(const MatchedItemMap &matchedData)
{
    m_matchWidget->appendMatchedData(matchedData);
}

void ExhibitionWidget::onSearchCompleted()
{
    m_matchWidget->onSearchCompleted();
}

void ExhibitionWidget::previewItem(const QString &searchGroupName, const MatchedItem &item)
{
    Q_ASSERT(m_previewWidget);
    Q_ASSERT(m_hLayout);

    // ????????????????????????web?????????????????????????????????????????????
    if (!Utils::canPreview(searchGroupName) || item.name.isEmpty()) {
        m_previewWidget->hide();

        // ??????????????????????????????????????????0, ????????????????????????????????????????????????
        m_hLayout->setContentsMargins(MARGIN_SIZE, 0, 0, MARGIN_SIZE);

        emit sigPreviewStateChanged(false);

        return;
    }

    // ???????????????????????????????????????????????????????????????
    m_hLayout->setContentsMargins(MARGIN_SIZE, 0, MARGIN_SIZE, MARGIN_SIZE);

    // ??????????????????????????????????????????
    m_previewWidget->previewItem(item);

    emit sigPreviewStateChanged(true);
}

void ExhibitionWidget::initUi()
{
    m_hLayout = new QHBoxLayout(this);
    m_hLayout->setContentsMargins(MARGIN_SIZE, 0, 0, MARGIN_SIZE);
    m_hLayout->setSpacing(0);
    this->setLayout(m_hLayout);

    m_matchWidget = new MatchWidget(this);
    m_matchWidget->setFocusPolicy(Qt::NoFocus);
    m_hLayout->addWidget(m_matchWidget);

    DVerticalLine* vLine = new DVerticalLine(this);
    m_hLayout->addWidget(vLine);

    m_previewWidget = new PreviewWidget(this);
    m_hLayout->addWidget(m_previewWidget);
}

void ExhibitionWidget::initConnect()
{
    Q_ASSERT(m_matchWidget);

    connect(m_matchWidget, &MatchWidget::sigCurrentItemChanged, this, &ExhibitionWidget::sigCurrentItemChanged);
    connect(m_matchWidget, &MatchWidget::sigShowNoContent, this, &ExhibitionWidget::sigShowNoContent);
    connect(m_matchWidget, &MatchWidget::sigCloseWindow, this, &ExhibitionWidget::sigCloseWindow);
    connect(m_matchWidget, &MatchWidget::sigCurrentItemChanged, this, &ExhibitionWidget::previewItem);

    connect(this, &ExhibitionWidget::sigPreviewStateChanged, m_matchWidget, &MatchWidget::onPreviewStateChanged);
}

void ExhibitionWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
}
