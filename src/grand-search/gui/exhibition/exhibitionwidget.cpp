// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define MARGIN_SIZE 10

ExhibitionWidgetPrivate::ExhibitionWidgetPrivate(ExhibitionWidget *parent)
    : q_p(parent)
{
}

ExhibitionWidget::ExhibitionWidget(QWidget *parent)
    : DWidget(parent), d_p(new ExhibitionWidgetPrivate(this))
{
    qCDebug(logGrandSearch) << "Creating ExhibitionWidget";
    initUi();
    initConnect();
    qCDebug(logGrandSearch) << "ExhibitionWidget created successfully";
}

ExhibitionWidget::~ExhibitionWidget()
{
    qCDebug(logGrandSearch) << "Destroying ExhibitionWidget";
}

void ExhibitionWidget::clearData()
{
    Q_ASSERT(m_matchWidget);

    qCDebug(logGrandSearch) << "Clearing exhibition data";
    m_matchWidget->clearMatchedData();
    m_previewWidget->hide();
    m_vLine->hide();
}

void ExhibitionWidget::setSearchKeyword(const QString &keyword)
{
    Q_ASSERT(m_matchWidget);

    qCDebug(logGrandSearch) << "Setting search keyword for highlight:" << keyword;
    m_matchWidget->setSearchKeyword(keyword);
}

void ExhibitionWidget::onSelectNextItem()
{
    if (this->isHidden()) {
        qCDebug(logGrandSearch) << "Select next item ignored - Exhibition widget is hidden";
        return;
    }

    Q_ASSERT(m_matchWidget);

    qCDebug(logGrandSearch) << "Selecting next item in exhibition";
    m_matchWidget->selectNextItem();
}

void ExhibitionWidget::onSelectPreviousItem()
{
    if (this->isHidden()) {
        qCDebug(logGrandSearch) << "Select previous item ignored - Exhibition widget is hidden";
        return;
    }

    Q_ASSERT(m_matchWidget);

    qCDebug(logGrandSearch) << "Selecting previous item in exhibition";
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
    qCDebug(logGrandSearch) << "Appending matched data - Groups:" << matchedData.size();
    m_matchWidget->appendMatchedData(matchedData);
}

void ExhibitionWidget::onSearchCompleted()
{
    qCDebug(logGrandSearch) << "Search completed - Processing final results";
    m_matchWidget->onSearchCompleted();
}

void ExhibitionWidget::previewItem(const QString &searchGroupName, const MatchedItem &item)
{
    Q_ASSERT(m_previewWidget);
    Q_ASSERT(m_hLayout);

    // 搜索类型为空，或web搜索、应用、设置等，不显示预览
    if (!Utils::canPreview(searchGroupName) || item.name.isEmpty()) {
        qCDebug(logGrandSearch) << "Hiding preview - Group:" << searchGroupName;
        m_previewWidget->hide();
        m_vLine->hide();

        // 不显示预览界面，右侧空隙需为0, 用来贴着主界面右侧显示滚动条区域
        m_hLayout->setContentsMargins(MARGIN_SIZE, 0, 0, MARGIN_SIZE);

        emit sigPreviewStateChanged(false);

        return;
    }

    qCDebug(logGrandSearch) << "Showing preview - Group:" << searchGroupName
                            << "Item:" << item.name;

    // 显示预览界面，主界面右侧需要与左侧间隙相同
    m_hLayout->setContentsMargins(MARGIN_SIZE, 0, MARGIN_SIZE, MARGIN_SIZE);

    // 预览界面预览选择的匹配结果项
    m_previewWidget->previewItem(item);
    m_vLine->show();

    emit sigPreviewStateChanged(true);
}

void ExhibitionWidget::initUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    m_authPromptWidget = new AuthPromptWidget(this);
    m_authPromptWidget->installEventFilter(this);
    vLayout->addWidget(m_authPromptWidget);

    m_hLayout = new QHBoxLayout();
    m_hLayout->setContentsMargins(MARGIN_SIZE, 0, 0, MARGIN_SIZE);
    m_hLayout->setSpacing(0);

    m_matchWidget = new MatchWidget(this);
    m_matchWidget->setFocusPolicy(Qt::NoFocus);
    m_hLayout->addWidget(m_matchWidget);

    m_vLine = new DVerticalLine(this);
    m_hLayout->addWidget(m_vLine);

    m_previewWidget = new PreviewWidget(this);
    m_hLayout->addWidget(m_previewWidget);
    vLayout->addLayout(m_hLayout);
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

bool ExhibitionWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_authPromptWidget && (e->type() == QEvent::Show || e->type() == QEvent::Hide)) {
        // 授权提示可见时，预览区上方留出间隔，避免与提示控件分隔线过近
        auto margins = m_previewWidget->contentsMargins();
        margins.setTop(m_authPromptWidget->isVisible() ? MARGIN_SIZE : 0);
        m_previewWidget->setContentsMargins(margins);
    }

    return QWidget::eventFilter(obj, e);
}

void ExhibitionWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
}
