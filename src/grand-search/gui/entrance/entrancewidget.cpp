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
#include "gui/datadefine.h"

//调试使用，最后发布时需删除todo
#include "gui/exhibition/matchresult/groupwidget.h"

#include <DSearchEdit>
#include <DStyle>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QAction>
#include <QTimer>
#include <QGuiApplication>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static const uint DelayReponseTime          = 50;       // 输入延迟搜索时间
static const uint EntraceWidgetWidth        = 740;      // 搜索界面宽度度
static const uint EntraceWidgetHeight       = 48;       // 搜索界面高度
static const uint WidgetMargins             = 10;       // 界面边距

EntranceWidgetPrivate::EntranceWidgetPrivate(EntranceWidget *parent)
    : q_p(parent)
{
    m_delayChangeTimer = new QTimer(this);
    m_delayChangeTimer->setSingleShot(true);
    m_delayChangeTimer->setInterval(DelayReponseTime);

    connect(m_delayChangeTimer, &QTimer::timeout, this, &EntranceWidgetPrivate::notifyTextChanged);
}

void EntranceWidgetPrivate::delayChangeText()
{
    m_delayChangeTimer->start();
}

void EntranceWidgetPrivate::notifyTextChanged()
{
    const QString &currentSearchText = m_searchEdit->text().trimmed();
    emit q_p->searchTextChanged(currentSearchText);
}

void EntranceWidgetPrivate::setLineEditFocus()
{
    m_searchEdit->lineEdit()->setFocus();
}

void EntranceWidgetPrivate::onFocusObjectChanged(QObject *obj)
{
    if (obj && obj != m_searchEdit->lineEdit()) {
        // 收到焦点改变信号后，必须等已有事件循环结束后，设置才能生效
        QTimer::singleShot(1, this, &EntranceWidgetPrivate::setLineEditFocus);
    }
}

EntranceWidget::EntranceWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new EntranceWidgetPrivate(this))
{
    initUI();
    initConnections();
}

EntranceWidget::~EntranceWidget()
{

}

void EntranceWidget::connectToController()
{

}

void EntranceWidget::paintEvent(QPaintEvent *event)
{
    // 调试使用，最后发布时需删除todo 用于显示搜索入口界面背景色
#ifdef SHOW_BACKCOLOR
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setBrush(Qt::darkCyan);
    painter.drawRect(rect());
#else
    DWidget::paintEvent(event);
#endif
}

void EntranceWidget::initUI()
{
    d_p->m_searchEdit = new DSearchEdit(this);

    // 搜索框界面布局设置
    // 必须对搜索框控件的边距和间隔设置为0,否则其内含的LineEdit不满足大小显示要求
    d_p->m_searchEdit->layout()->setSpacing(0);
    d_p->m_searchEdit->layout()->setMargin(0);
    d_p->m_searchEdit->lineEdit()->setFixedSize(EntraceWidgetWidth, EntraceWidgetHeight);

    // 搜索框提示信息设置
    d_p->m_searchEdit->setPlaceHolder(tr("Search"));
    d_p->m_searchEdit->setPlaceholderText(tr("input your word"));

    // 搜索界面布局设置
    d_p->m_mainLayout = new QHBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_searchEdit);
    // 根据设计图要求，设置边距和间隔
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->setMargin(WidgetMargins);

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

    // 焦点改变后，将焦点设置回输入框
    connect(qApp, &QGuiApplication::focusObjectChanged, d_p.data(), &EntranceWidgetPrivate::onFocusObjectChanged);
}
