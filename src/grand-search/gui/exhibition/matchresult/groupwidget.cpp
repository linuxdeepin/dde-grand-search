/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
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
#include "groupwidget_p.h"
#include "groupwidget.h"
#include "listview/grandsearchlistview.h"

#include <DLabel>
#include <DHorizontalLine>
#include <DApplicationHelper>

#include <QVBoxLayout>

using namespace GrandSearch;

GroupWidgetPrivate::GroupWidgetPrivate(GroupWidget *parent)
    : q_p(parent)
{

}

GroupWidget::GroupWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new GroupWidgetPrivate(this))
{
    initUi();
    initConnect();

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

GroupWidget::~GroupWidget()
{

}

void GroupWidget::setGroupName(const QString &groupHash)
{
    if (nullptr == m_GroupLabel)
        return;

    QString groupName = GroupWidget::getGroupName(groupHash);
    QString groupObjName = GroupWidget::getGroupObjName(groupHash);

    m_GroupLabel->setObjectName(groupName);

    if (m_GroupLabel)
        m_GroupLabel->setText(groupName);

    qDebug() << QString("GroupWidget::setGroupName groupName:[%1] groupHash:[%2]").arg(groupName).arg(groupHash);
}

void GroupWidget::setMatchedItems(const MatchedItems &items)
{
    qDebug() << QString("GroupWidget::setMatchedItems group:%1").arg(m_GroupLabel->text());

    if (m_ListView)
        m_ListView->setMatchedItems(items);

    layout();
}

void GroupWidget::showHorLine(bool bShow)
{
    if (m_Line)
        m_Line->setVisible(bShow);

    layout();
}

bool GroupWidget::isHorLineVisilbe()
{
    if (m_Line)
        m_Line->isVisible();

    return false;
}

GrandSearchListview *GroupWidget::getListView()
{
    return m_ListView;
}

int GroupWidget::itemCount()
{
    if (m_ListView)
        return m_ListView->rowCount();

    return 0;
}

void GroupWidget::reLayout()
{
    if (nullptr == m_ListView)
        return;

    m_ListView->setFixedHeight(m_ListView->rowCount() * 34);

    int nHeight = 0;
    if (m_GroupLabel) {
        qDebug() << QString("GroupWidget::reLayout m_GroupLabel height:[%1]").arg(m_GroupLabel->height());
        nHeight += m_GroupLabel->height();
    }
    if (m_ListView) {
        qDebug() << QString("GroupWidget::reLayout m_listView height:[%1]").arg(m_ListView->height());
        nHeight += m_ListView->height();
    }
    if (m_Line && !m_Line->isHidden()) {
        qDebug() << QString("GroupWidget::reLayout m_Line height:[%1]").arg(m_Line->height());
        nHeight += m_Line->height();
    }

    this->setFixedHeight(nHeight);

    layout();

    qDebug() << QString("GroupWidget::reLayout groupWidget height:[%1]").arg(this->height());
}

QString GroupWidget::groupName()
{
    if (m_GroupLabel)
        return m_GroupLabel->text();

    return "";
}

QString GroupWidget::getGroupName(const QString &groupHash)
{
    QString strName = groupHash;

    if (GroupHash_App == groupHash)
        strName = GroupName_App;
    else if (GroupHash_Folder == groupHash)
        strName = GroupName_Folder;
    else if (GroupHash_File == groupHash)
        strName = GroupName_File;

    return strName;
}

QString GroupWidget::getGroupObjName(const QString &groupHash)
{
    QString strObjName = groupHash;

    if (GroupHash_App == groupHash)
        strObjName = GroupObjName_App;
    else if (GroupHash_Folder == groupHash)
        strObjName = GroupObjName_Folder;
    else if (GroupHash_File == groupHash)
        strObjName = GroupObjName_File;

    return strObjName;
}

void GroupWidget::initUi()
{
    // 获取设置当前窗口文本颜色
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, QColor("#414D68 "));

    // 组列表内控件沿垂直方向布局
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setContentsMargins(0, 0, 10, 0);
    m_vLayout->setSpacing(0);
    this->setLayout(m_vLayout);

    // 组名标签
    m_GroupLabel = new DLabel(tr(""), this);
    m_GroupLabel->setFixedHeight(34);
    m_GroupLabel->setPalette(labelPalette);
    m_GroupLabel->setContentsMargins(0, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_GroupLabel, DFontSizeManager::T8, QFont::Normal);

    // 组内结果列表
    m_ListView = new GrandSearchListview(this);
    m_ListView->setMinimumWidth(380);

    // 分割线
    m_Line = new DHorizontalLine;
    m_Line->setFrameShadow(DHorizontalLine::Raised);
    m_Line->setLineWidth(1);

    // 控件放置到布局内
    m_vLayout->addWidget(m_GroupLabel, 1);
    m_vLayout->addWidget(m_ListView);
    m_vLayout->addWidget(m_Line);
}

void GroupWidget::initConnect()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &GroupWidget::setThemeType);
}

void GroupWidget::paintEvent(QPaintEvent *event)
{
#ifdef SHOW_BACKCOLOR
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setBrush(Qt::green);
    painter.drawRect(rect());
#else
    DWidget::paintEvent(event);
#endif
}

void GroupWidget::setThemeType(int type)
{
    Q_UNUSED(type);

    // 后续等待UI确认
//    // 窗口主题切换，浅色到深色互切，列表内文本颜色要跟着改变
//    QPalette labelPalette;
//    if (type == 2) {
//        labelPalette.setColor(QPalette::Text, QColor("#FFF0F5 "));
//    } else {
//        labelPalette.setColor(QPalette::Text, QColor("#414D68 "));
//    }
//    m_ListView->setPalette(labelPalette);
}


