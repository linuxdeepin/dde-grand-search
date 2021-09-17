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
#include "generaltoolbar.h"

#include <DPushButton>
#include <DVerticalLine>
#include <DGuiApplicationHelper>

#include <QHBoxLayout>
#include <QGuiApplication>

#define TOOLBTN_WIDTH           100
#define TOOLBAR_HEIGHT          36
#define TOOLBAR_BOTTOM_MARGIN   10

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

IconButton::IconButton(QWidget *parent)
    : QToolButton(parent)
{
    setFixedWidth(TOOLBTN_WIDTH);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QColor textColor = QColor(82, 106, 127, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 1));
    QPalette pa = palette();
    pa.setColor(QPalette::ButtonText, textColor);
    setPalette(pa);
}

GeneralToolBar::GeneralToolBar(QWidget *parent)
    : DWidget(parent)
{
    initUi();
    initConnect();
}

void GeneralToolBar::initUi()
{
    setFixedHeight(TOOLBAR_HEIGHT + TOOLBAR_BOTTOM_MARGIN); //高36 + 下边距10

    m_hMainLayout = new QHBoxLayout(this);
    //下边距10
    m_hMainLayout->setContentsMargins(0, 0, 0, TOOLBAR_BOTTOM_MARGIN);
    m_hMainLayout->setSpacing(0);

    m_openBtn = new IconButton(this);
    m_openBtn->setText(tr("Open"));
    m_openBtn->setIcon(QIcon(":/icons/open.svg"));

    m_openPathBtn = new IconButton(this);
    m_openPathBtn->setText(tr("Open Path"));
    m_openPathBtn->setIcon(QIcon(":/icons/openpath.svg"));

    m_copyPathBtn = new IconButton(this);
    m_copyPathBtn->setText(tr("Copy Path"));
    m_copyPathBtn->setIcon(QIcon(":/icons/copypath.svg"));

    m_vLine1 = new DVerticalLine(this);
    m_vLine2 = new DVerticalLine(this);

    m_hMainLayout->addWidget(m_openBtn);
    m_hMainLayout->addWidget(m_vLine1);
    m_hMainLayout->addWidget(m_openPathBtn);
    m_hMainLayout->addWidget(m_vLine2);
    m_hMainLayout->addWidget(m_copyPathBtn);

    this->setLayout(m_hMainLayout);
}

void GeneralToolBar::initConnect()
{
    Q_ASSERT(m_openBtn);
    Q_ASSERT(m_openPathBtn);
    Q_ASSERT(m_copyPathBtn);

    connect(m_openBtn, &DPushButton::clicked, this, &GeneralToolBar::sigOpenClicked);
    connect(m_openPathBtn, &DPushButton::clicked, this, &GeneralToolBar::sigOpenPathClicked);
    connect(m_copyPathBtn, &DPushButton::clicked, this, &GeneralToolBar::sigCopyPathClicked);
}
