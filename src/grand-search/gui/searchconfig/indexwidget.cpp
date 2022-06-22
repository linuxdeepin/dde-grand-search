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

#include "indexwidget.h"
#include "blacklistwidget.h"

#include <DFontSizeManager>
#include <QLabel>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
IndexWidget::IndexWidget(QWidget *parent)
    : DWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_blackListWidget = new BlackListWidget(this);
    m_groupLabel = new QLabel(tr("index"));
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);

    m_mainLayout->addWidget(m_groupLabel);
    m_mainLayout->addWidget(m_blackListWidget);
}

IndexWidget::~IndexWidget()
{

}
