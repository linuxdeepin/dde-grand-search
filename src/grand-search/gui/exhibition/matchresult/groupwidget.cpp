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
#include "global/accessibility/acintelfunctions.h"

#include <DLabel>
#include <DPushButton>
#include <DHorizontalLine>
#include <DApplicationHelper>

#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace GrandSearch;

#define ListItemHeight            36       // 列表行高
#define GroupLabelHeight          28       // 组标签高度
#define MoreBtnMaxHeight          25       // 查看更多按钮最大高度
#define LayoutMagrinSize          10       // 布局边距
#define SpacerWidth               40       // 弹簧宽度
#define SpacerHeight              20       // 弹簧高度

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
}

GroupWidget::~GroupWidget()
{

}

void GroupWidget::setGroupName(const QString &groupName)
{
    Q_ASSERT(m_groupLabel);
    Q_ASSERT(m_listView);

    m_groupLabel->setText(groupName);

    AC_SET_ACCESSIBLE_NAME(m_listView, groupName);
}

void GroupWidget::appendMatchedItems(const MatchedItems &newItems, const QString& searchGroupName)
{
    Q_UNUSED(searchGroupName)

    if (Q_UNLIKELY(newItems.isEmpty()))
        return;

    // 结果列表未展开
    if (!m_bListExpanded) {

        // 新来数据先放入缓存中
        m_cacheItems << newItems;

        // 显示不足5个，连带新增数据一起重排
        if (GROUP_MAX_SHOW != m_listView->rowCount()) {

            // 清空列表数据，将已显示数据还原到各自缓存中
            m_cacheItems << m_firstFiveItems;
            m_firstFiveItems.clear();

            // 拉通重排缓存中的匹配结果
            Utils::sort(m_cacheItems);
            for (int i = 0; i < GROUP_MAX_SHOW; i++) {
                if (!m_cacheItems.isEmpty())
                    m_firstFiveItems.push_back(m_cacheItems.takeFirst());
            }
            m_listView->setMatchedItems(m_firstFiveItems);
        }

        // 缓存中有数据，显示'查看更多'按钮
        m_viewMoreButton->setVisible(!m_cacheItems.isEmpty());
    } else {
        // 结果列表已展开，已经显示的数据保持不变，仅对新增数据排序，然后追加到列表末尾
        MatchedItems& tempNewItems = const_cast<MatchedItems&>(newItems);
        Utils::sort(tempNewItems);
        m_listView->addRows(tempNewItems);
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

    return  m_line->isVisible();
}

GrandSearchListView *GroupWidget::getListView()
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
        nHeight += (m_listView->currentIndex().row() + 1) * ListItemHeight;
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

void GroupWidget::setSearchGroupName(const QString &searchGroupName)
{
    m_searchGroupName = searchGroupName;
}

QString GroupWidget::searchGroupName() const
{
    return m_searchGroupName;
}

QString GroupWidget::groupName() const
{
    Q_ASSERT(m_groupLabel);

    return m_groupLabel->text();
}

QString GroupWidget::convertDisplayName(const QString &searchGroupName)
{
    static const QHash<QString, QString> groupDisplayName{
        {GRANDSEARCH_GROUP_APP, GroupName_App}
        , {GRANDSEARCH_GROUP_SETTING, GroupName_Setting}
        , {GRANDSEARCH_GROUP_FILE_VIDEO, GroupName_Video}
        , {GRANDSEARCH_GROUP_FILE_AUDIO, GroupName_Audio}
        , {GRANDSEARCH_GROUP_FILE_PICTURE, GroupName_Picture}
        , {GRANDSEARCH_GROUP_FILE_DOCUMNET, GroupName_Document}
        , {GRANDSEARCH_GROUP_FOLDER, GroupName_Folder}
        , {GRANDSEARCH_GROUP_FILE, GroupName_File}
        , {GRANDSEARCH_GROUP_WEB, GroupName_Web}
    };

    return groupDisplayName.value(searchGroupName, searchGroupName);
}

void GroupWidget::initUi()
{
    // 获取设置当前窗口文本颜色
    QColor groupTextColor = QColor(0, 0, 0, static_cast<int>(255*0.6));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        groupTextColor = QColor(255, 255, 255, static_cast<int>(255*0.6));
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, groupTextColor);

    // 组列表内控件沿垂直方向布局
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setContentsMargins(0, 0, LayoutMagrinSize, 0);
    m_vLayout->setSpacing(0);
    this->setLayout(m_vLayout);

    // 组名标签
    m_groupLabel = new DLabel("", this);
    m_groupLabel->setFixedHeight(GroupLabelHeight);
    m_groupLabel->setPalette(labelPalette);
    m_groupLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_groupLabel->setContentsMargins(0, 0, 0, 0);

    QFont groupLabelFont = m_groupLabel->font();
    groupLabelFont.setWeight(QFont::Normal);
    groupLabelFont = DFontSizeManager::instance()->get(DFontSizeManager::T8, groupLabelFont);
    m_groupLabel->setFont(groupLabelFont);

    // 查看更多按钮
    m_viewMoreButton = new DPushButton(tr("More"), this);
    m_viewMoreButton->setMaximumHeight(MoreBtnMaxHeight);
    m_viewMoreButton->setFlat(true);

    // 设置查看按钮文本颜色
    QColor moreTextColor = QColor(0, 0, 0, static_cast<int>(255*0.4));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        moreTextColor = QColor(255, 255, 255, static_cast<int>(255*0.4));
    QPalette palette = m_viewMoreButton->palette();
    palette.setColor(QPalette::ButtonText, moreTextColor);
    // 使'查看更多'按钮按下时，背景色变淡
    QBrush brush(QColor(0,0,0,0));
    palette.setBrush(QPalette::Active, QPalette::Button, brush);
    palette.setBrush(QPalette::Active, QPalette::Light, brush);
    palette.setBrush(QPalette::Active, QPalette::Midlight, brush);
    palette.setBrush(QPalette::Active, QPalette::Dark, brush);
    palette.setBrush(QPalette::Active, QPalette::Mid, brush);
    m_viewMoreButton->setPalette(palette);
    DFontSizeManager::instance()->bind(m_viewMoreButton, DFontSizeManager::T8, QFont::Normal);
    QFont fontMoreBtn = m_viewMoreButton->font();
    fontMoreBtn.setWeight(QFont::Normal);
    fontMoreBtn = DFontSizeManager::instance()->get(DFontSizeManager::T8, fontMoreBtn);
    m_viewMoreButton->setFont(fontMoreBtn);
    m_viewMoreButton->hide();

    // 组列表标题栏布局
    m_hTitelLayout = new QHBoxLayout();
    m_hTitelLayout->setContentsMargins(LayoutMagrinSize,0,0,0);
    m_hTitelLayout->setSpacing(0);
    m_hTitelLayout->addWidget(m_groupLabel);
    m_hTitelLayout->addSpacerItem(new QSpacerItem(SpacerWidth,SpacerHeight,QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_hTitelLayout->addWidget(m_viewMoreButton);

    // 组内结果列表
    m_listView = new GrandSearchListView(this);
    m_listView->setFocusPolicy(Qt::NoFocus);

    // 分割线
    m_line = new DHorizontalLine;

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

    connect(m_viewMoreButton, &DPushButton::clicked, this, &GroupWidget::onMoreBtnClcked);
}

void GroupWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
}

void GroupWidget::onMoreBtnClcked()
{
    Q_ASSERT(m_listView);
    Q_ASSERT(m_viewMoreButton);

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


