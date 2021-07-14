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
#include "entrancewidget_p.h"
#include "entrancewidget.h"

#include <DSearchEdit>
#include <DStyle>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QAction>
#include <QTimer>

DWIDGET_USE_NAMESPACE

EntranceWidgetPrivate::EntranceWidgetPrivate(EntranceWidget *parent)
    : q_p(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(50);

    connect(m_timer, &QTimer::timeout, this, &EntranceWidgetPrivate::notifyTextChanged);
}

void EntranceWidgetPrivate::delayChangeText()
{
    m_timer->start();
}

void EntranceWidgetPrivate::notifyTextChanged()
{
    const QString &currentSearchText = m_searchEdit->text();
    emit q_p->searchTextChanged(currentSearchText);
}

EntranceWidget::EntranceWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , d_p(new EntranceWidgetPrivate(this))
{
    initUI();
    initConnections();
}

EntranceWidget::~EntranceWidget()
{

}

void EntranceWidget::initUI()
{
    d_p->m_searchEdit = new DSearchEdit(this);
    d_p->m_mainLayout = new QHBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_searchEdit);
    this->setLayout(d_p->m_mainLayout);

    // todo 搜索框图标与清除按钮定制
//    QAction *leftaction = d_p->m_searchEdit->findChild<QAction *>("_d_search_leftAction");
//    if (leftaction) {
//        leftaction->setIcon(QIcon(":/icons/skin/icons/search_36px.svg"));
//    }
//    QAction *clearAction = d_p->m_searchEdit->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"));
//    if (clearAction) {
//        clearAction->setIcon(QIcon(":/icons/skin/icons/clear_36px.svg"));
//    }

//    d_p->m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//    d_p->m_searchEdit->lineEdit()->setFixedSize(SEARCHEIT_WIDTH, SEARCHEIT_HEIGHT);
//    this->setMouseTracking(true);
//    DStyle::setFocusRectVisible(d_p->m_searchEdit->lineEdit(), true);
//    setFocusPolicy(Qt::NoFocus);


}

void EntranceWidget::initConnections()
{
    // 输入改变时重置定时器，避免短时间内发起大量无效调用
    connect(d_p->m_searchEdit, &DSearchEdit::textChanged, d_p.data(), &EntranceWidgetPrivate::delayChangeText);
}
