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
#include "utils/utils.h"

#include <DLabel>
#include <DPushButton>
#include <DHorizontalLine>
#include <DApplicationHelper>

#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace GrandSearch;

#define GROUP_MAX_SHOW 5
#define ListItemHeight            36       // 列表行高
#define GroupLabelHeight          28       // 组标签高度
#define ViewMoreBtnWidth          80       // 查看更多按钮宽度
#define ViewMoreBtnHeight         17       // 查看更多按钮高度
#define LayoutMagrinSize          10       // 布局边距
#define SpacerWidth               40       // 弹簧宽度
#define SpacerHeight              20       // 弹簧高度
#define MaxFileShowCount          100      // 文件(夹)最大显示数

GroupWidgetPrivate::GroupWidgetPrivate(GroupWidget *parent)
    : q_p(parent)
{

}

GroupWidget::GroupWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new GroupWidgetPrivate(this))
{
    m_firstFiveItems.clear();
    m_restShowItems.clear();
    m_cacheItems.clear();

    initUi();
    initConnect();

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

GroupWidget::~GroupWidget()
{
    qDebug() << QString("groupWidget %1 destructed....").arg(m_groupName);
}

void GroupWidget::setGroupName(const QString &groupHash)
{
    Q_ASSERT(m_groupLabel);

    const QString &groupName = GroupWidget::getGroupName(groupHash);
    const QString &groupObjName = GroupWidget::getGroupObjName(groupHash);

    m_groupName = groupName;

    m_groupLabel->setObjectName(groupObjName);
    m_groupLabel->setText(groupName);
}

void GroupWidget::appendMatchedItems(const MatchedItems &newItems)
{
    if (Q_UNLIKELY(newItems.isEmpty()))
        return;

    // 结果列表未展开
    if (!m_bListExpanded) {

        // 新来的数据先放入缓存中
        m_cacheItems << newItems;

        // 显示结果不足5个，连带新增数据一起重新排序
        if (m_firstFiveItems.size() < GROUP_MAX_SHOW) {

            m_cacheItems << m_firstFiveItems;
            Utils::sort(m_cacheItems);

            m_firstFiveItems.clear();
            for (int i = 0; i < GROUP_MAX_SHOW; i++) {
                if (!m_cacheItems.isEmpty())
                    m_firstFiveItems.push_back(m_cacheItems.takeFirst());
            }

            m_listView->setMatchedItems(m_firstFiveItems);
        }

        // 若类目为文件或文件夹，当前结果结果数超过100个，仅保留前100个，保证结果展示数在100以内
        if (m_groupName == GroupName_Folder
         || m_groupName == GroupName_File) {
            int nRemoveCount = m_cacheItems.size() + m_firstFiveItems.size() - MaxFileShowCount;
            if (nRemoveCount > 0) {
                for (int i = 0; i < nRemoveCount; i++)
                    m_cacheItems.removeLast();
            }

        }
        // 缓存中有数据，显示'查看更多'按钮
        m_viewMoreButton->setVisible(!m_cacheItems.isEmpty());
    }
    else {
        // 结果列表已展开，已经显示的数据保持不变，仅对新增数据排序，然后追加到列表末尾
        MatchedItems& tempNewItems = const_cast<MatchedItems&>(newItems);
        Utils::sort(tempNewItems);

        // 若类目为文件或文件夹，保证结果数据在100以内
        if (m_groupName == GroupName_Folder
         || m_groupName == GroupName_File) {
            for (int i = 0; i < tempNewItems.size() && m_listView->rowCount() < MaxFileShowCount; i++) {
                m_listView->addRow(tempNewItems[i]);
            }
        } else {
            m_listView->addRows(tempNewItems);
        }
    }

    layout();
}

void GroupWidget::showHorLine(bool bShow)
{
    Q_ASSERT(m_line);

    m_line->setVisible(bShow);

    layout();
}

bool GroupWidget::isHorLineVisilbe()
{
    Q_ASSERT(m_line);

    m_line->isVisible();

    return false;
}

GrandSearchListview *GroupWidget::getListView()
{
    return m_listView;
}

int GroupWidget::itemCount()
{
    Q_ASSERT(m_listView);

    return m_listView->rowCount();
}

int GroupWidget::getCurSelectHeight()
{
    Q_ASSERT(m_listView);

    int nHeight = 0;
    if (m_listView->currentIndex().isValid()) {
        nHeight += m_groupLabel->height();
        nHeight += (m_listView->currentIndex().row()+1) * ListItemHeight;

        if (!m_line->isHidden())
            nHeight += m_line->height();
    }

    return nHeight;
}

void GroupWidget::reLayout()
{
    Q_ASSERT(m_listView);
    Q_ASSERT(m_groupLabel);
    Q_ASSERT(m_line);

    m_listView->setFixedHeight(m_listView->rowCount() * ListItemHeight);

    int nHeight = 0;
    nHeight += m_groupLabel->height();
    nHeight += m_listView->height();

    if (!m_line->isHidden()) {
        nHeight += m_line->height();
        nHeight += LayoutMagrinSize;
        m_vContentLayout->setSpacing(10);
    } else {
        m_vContentLayout->setSpacing(0);
    }

    this->setFixedHeight(nHeight);

    layout();
}

void GroupWidget::clear()
{
    Q_ASSERT(m_listView);

    m_firstFiveItems.clear();
    m_restShowItems.clear();
    m_cacheItems.clear();
    m_bListExpanded = false;

    m_listView->clear();
    setVisible(false);
}

bool GroupWidget::isAppendDataAllow()
{
    // 当文件(夹)总数超过100条记录时，不允许追加
    if (m_groupName == GroupName_Folder
     || m_groupName == GroupName_File) {
        return itemCount() < MaxFileShowCount;
    }

    return true;
}

QString GroupWidget::groupName()
{
    Q_ASSERT(m_groupLabel);

    return m_groupLabel->text();
}

QString GroupWidget::getGroupName(const QString &groupHash)
{
    QString strName = groupHash;

    if (GRANDSEARCH_GROUP_APP == groupHash)
        strName = GroupName_App;
    else if (GRANDSEARCH_GROUP_FOLDER == groupHash)
        strName = GroupName_Folder;
    else if (GRANDSEARCH_GROUP_FILE == groupHash)
        strName = GroupName_File;

    return strName;
}

QString GroupWidget::getGroupObjName(const QString &groupHash)
{
    QString strObjName = groupHash;

    if (GRANDSEARCH_GROUP_APP == groupHash)
        strObjName = GroupObjName_App;
    else if (GRANDSEARCH_GROUP_FOLDER == groupHash)
        strObjName = GroupObjName_Folder;
    else if (GRANDSEARCH_GROUP_FILE == groupHash)
        strObjName = GroupObjName_File;

    return strObjName;
}

void GroupWidget::initUi()
{
    // 获取设置当前窗口文本颜色
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, QColor(0,0,0, static_cast<int>(255*0.6)));

    // 组列表内控件沿垂直方向布局
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setContentsMargins(0, 0, LayoutMagrinSize, 0);
    m_vLayout->setSpacing(0);
    this->setLayout(m_vLayout);

    // 组名标签
    m_groupLabel = new DLabel(tr(""), this);
    m_groupLabel->setFixedHeight(GroupLabelHeight);
    m_groupLabel->setPalette(labelPalette);
    m_groupLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_groupLabel->setContentsMargins(0, 0, 0, 0);
    //DFontSizeManager::instance()->bind(m_GroupLabel, DFontSizeManager::T8, QFont::Normal);

    // 查看更多按钮
    m_viewMoreButton = new DPushButton(tr("More"), this);
    m_viewMoreButton->setFixedSize(ViewMoreBtnWidth,ViewMoreBtnHeight);
    m_viewMoreButton->setFlat(true);
    //DFontSizeManager::instance()->bind(m_viewMoreButton, DFontSizeManager::T8, QFont::Normal);

    QPalette palette = m_viewMoreButton->palette();
    // 设置查看按钮文本颜色
    palette.setColor(QPalette::ButtonText, QColor(0,0,0, static_cast<int>(255*0.4)));
    // 使'查看更多'按钮按下时，背景色变淡
    QBrush brush(QColor(0,0,0,0));
    palette.setBrush(QPalette::Active, QPalette::Button, brush);
    palette.setBrush(QPalette::Active, QPalette::Light, brush);
    palette.setBrush(QPalette::Active, QPalette::Midlight, brush);
    palette.setBrush(QPalette::Active, QPalette::Dark, brush);
    palette.setBrush(QPalette::Active, QPalette::Mid, brush);
    m_viewMoreButton->setPalette(palette);
    DFontSizeManager::instance()->bind(m_viewMoreButton, DFontSizeManager::T8, QFont::Normal);
    m_viewMoreButton->hide();

    // 组列表标题栏布局
    m_hTitelLayout = new QHBoxLayout();
    m_hTitelLayout->setContentsMargins(LayoutMagrinSize,0,0,0);
    m_hTitelLayout->setSpacing(0);
    m_hTitelLayout->addWidget(m_groupLabel);
    m_hTitelLayout->addSpacerItem(new QSpacerItem(SpacerWidth,SpacerHeight,QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_hTitelLayout->addWidget(m_viewMoreButton);

    // 组内结果列表
    m_listView = new GrandSearchListview(this);

    // 分割线
    m_line = new DHorizontalLine;
    m_line->setFrameShadow(DHorizontalLine::Raised);
    m_line->setLineWidth(1);

    // 列表和分割线放到内容布局内
    m_vContentLayout = new QVBoxLayout();
    m_vContentLayout->setMargin(0);
    m_vContentLayout->setSpacing(0);
    m_vContentLayout->addWidget(m_listView);
    m_vContentLayout->addWidget(m_line);

    // 标题栏布局和内容布局放到主布局内
    m_vLayout->addLayout(m_hTitelLayout);
    m_vLayout->addLayout(m_vContentLayout);
}

void GroupWidget::initConnect()
{
    Q_ASSERT(m_viewMoreButton);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &GroupWidget::setThemeType);

    connect(m_viewMoreButton, &DPushButton::clicked, this, &GroupWidget::onMoreBtnClcked);
}

void GroupWidget::paintEvent(QPaintEvent *event)
{
// 调试使用，最后发布时需删除todo
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

void GroupWidget::onMoreBtnClcked()
{
    Q_ASSERT(m_listView);
    Q_ASSERT(m_viewMoreButton);


    // '查看更多'被点击，列表被展开
    // 将缓存中的数据转移到剩余显示结果中
    m_restShowItems << m_cacheItems;
    Utils::sort(m_restShowItems);

    // 剩余显示结果追加显示到列表中
    m_listView->addRows(m_restShowItems);

    // 清空缓存中数据
    m_cacheItems.clear();

    reLayout();

    emit showMore();
    m_viewMoreButton->hide();

    m_bListExpanded = true;
}


