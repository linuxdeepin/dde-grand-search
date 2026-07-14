// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwidget.h"
#include "utils/utils.h"
#include "utils/highlightprovider.h"
#include "generalpreviewplugin.h"
#include "generalwidget/detailwidget.h"
#include "generalwidget/generaltoolbar.h"
#include "generalwidget/aitoolbar.h"
#include "pluginproxy.h"
#include "global/builtinsearch.h"

#include <DScrollArea>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <DGuiApplicationHelper>
#else
#include <DApplicationHelper>
#endif
#include <DFrame>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollBar>
#include <QToolButton>
#include <QClipboard>
#include <QLoggingCategory>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define CONTENT_WIDTH           372

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

PreviewWidget::PreviewWidget(QWidget *parent)
    : DWidget(parent)
    , m_proxy(new PluginProxy(this))
{
    qCDebug(logGrandSearch) << "Creating PreviewWidget";

    m_generalPreview = QSharedPointer<PreviewPlugin>(new GeneralPreviewPlugin());
    m_generalPreview->init(m_proxy);

    initUi();
    initConnect();

    qCDebug(logGrandSearch) << "PreviewWidget created successfully";
}

PreviewWidget::~PreviewWidget()
{
    qCDebug(logGrandSearch) << "Destroying PreviewWidget";

    // 解除当前预览插件界面与预览主界面父子窗口关系，所有预览插件界面统一由插件管理类析构函数释放
    clearLayoutWidgets();
    if (m_vSpaceItem) {
        delete m_vSpaceItem;
        m_vSpaceItem = nullptr;
    }
}

bool PreviewWidget::previewItem(const MatchedItem &item)
{
    qCDebug(logGrandSearch) << "Previewing item:" << item.name << "Type:" << item.type;

    m_item = item;

    QSharedPointer<PreviewPlugin> preview = QSharedPointer<PreviewPlugin>(m_pluginManager.getPreviewPlugin(item));

    if (!preview) {
        qCDebug(logGrandSearch) << "Using general preview plugin";
        preview = m_generalPreview;
    } else {
        qCDebug(logGrandSearch) << "Using specific preview plugin";
        preview->init(m_proxy);
    }

    // 插件界面根据新来搜索结果刷新预览内容
    ItemInfo itemInfo;
    itemInfo[PREVIEW_ITEMINFO_ITEM] = item.item;
    itemInfo[PREVIEW_ITEMINFO_NAME] = item.name;
    itemInfo[PREVIEW_ITEMINFO_ICON] = item.icon;
    itemInfo[PREVIEW_ITEMINFO_TYPE] = item.type;
    itemInfo[PREVIEW_ITEMINFO_SEARCHER] = item.searcher;

    // Pass matched context from extra data
    QVariantHash extraHash = item.extra.toHash();
    if (extraHash.contains(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT)) {
        itemInfo[PREVIEW_ITEMINFO_MATCHEDCONTEXT] = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT).toString();
    }

    // Pass keywords for highlighting
    if (extraHash.contains(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS)) {
        itemInfo[PREVIEW_ITEMINFO_KEYWORDS] = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS).toStringList().join(":");
    }

    preview->previewItem(itemInfo);

    // 插件有变更， 更换新插件界面内容到主界面布局中
    if (preview != m_preview) {
        qCDebug(logGrandSearch) << "Switching preview plugin";

        // 清空主界面布局中原有预览插件界面内容
        clearLayoutWidgets();

        // 1. 更换新插件界面内容部件到主界面布局
        if (QWidget* contentWidget = preview->contentWidget()) {
            contentWidget->setFixedWidth(CONTENT_WIDTH);
            m_vMainLayout->addWidget(contentWidget);
            contentWidget->setVisible(true);
        }

        // 2. 添加详情属性部件到预览界面
        m_vMainLayout->addWidget(m_detailInfoWidget);

        // 文档类型的文件添加AI工具栏到预览界面
        m_vMainLayout->addWidget(m_aiToolBar);

        // 3. 添加垂直弹簧条，将工具栏部件置底到底部显示
        m_vMainLayout->addSpacerItem(m_vSpaceItem);

        if (preview->showToolBar()) {
            // 未定制工具栏，使用通用工具栏
            QWidget *toolBar = preview->toolBarWidget();
            if (!toolBar)
                toolBar = m_generalToolBar;
            Q_ASSERT(toolBar);
            m_vMainLayout->addWidget(toolBar);
        }

        m_preview = preview;
    }

    // 文档类型的文件添加AI工具栏到预览界面
    QString lowerName = item.name.toLower();
    bool isShowAiToolBar = lowerName.endsWith(".txt") || lowerName.endsWith(".doc") || lowerName.endsWith(".docx")  || lowerName.endsWith(".xls")
            || lowerName.endsWith(".xlsx") || lowerName.endsWith(".ppt") || lowerName.endsWith(".pptx") || lowerName.endsWith(".pdf");
    isShowAiToolBar = isShowAiToolBar && AiToolBar::checkUosAiInstalled();
    m_aiToolBar->setVisible(isShowAiToolBar);
    m_aiToolBar->setFilePath(item.item);

    bool hasDetailInfo = !preview->getAttributeDetailInfo().isEmpty();
    m_detailInfoWidget->setDetailInfoList(preview->getAttributeDetailInfo());
    if (isShowAiToolBar) {
        if (hasDetailInfo) {
            m_detailInfoWidget->setVisible(true);
            m_aiToolBar->setTopSpace(0);
        } else {
            m_detailInfoWidget->setVisible(false);
            m_aiToolBar->setTopSpace(10);
        }
        m_aiToolBar->adjustSize();
    } else {
        m_detailInfoWidget->setVisible(true);
    }

    // 内容部件高度控制：每次预览都重新计算
    if (QWidget* contentWidget = preview->contentWidget()) {
        contentWidget->setFixedWidth(CONTENT_WIDTH);
        if (preview->expandContent()) {
            int contentHeight = calculateContentHeight(hasDetailInfo, isShowAiToolBar);
            if (contentHeight > 0)
                contentWidget->setFixedHeight(contentHeight);
        } else {
            contentWidget->adjustSize();
        }
    }

    this->show();

    return true;
}

void PreviewWidget::initUi()
{
    m_vMainLayout = new QVBoxLayout(this);
    //不设置边距，由预览界面控制边距
    m_vMainLayout->setContentsMargins(0, 0, 0, 0);
    m_vMainLayout->setSpacing(0);

    m_detailInfoWidget = new DetailWidget(this);
    m_generalToolBar = new GeneralToolBar(this);
    m_aiToolBar = new AiToolBar(this);

    m_vSpaceItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    setFixedWidth(CONTENT_WIDTH);
}

void PreviewWidget::initConnect()
{
    connect(m_generalToolBar, &GeneralToolBar::sigOpenClicked, this, &PreviewWidget::onOpenClicked);
    connect(m_generalToolBar, &GeneralToolBar::sigOpenPathClicked, this, &PreviewWidget::onOpenpathClicked);
    connect(m_generalToolBar, &GeneralToolBar::sigCopyPathClicked, this, &PreviewWidget::onCopypathClicked);

    // 连接高亮内容获取完成信号
    connect(HighlightProvider::instance(), &HighlightProvider::highlightReady,
            this, &PreviewWidget::updateHighlightContent);
}

void PreviewWidget::clearLayoutWidgets()
{
    qCDebug(logGrandSearch) << "Clearing preview layout widgets";

    if (m_preview) {
        m_preview->stopPreview();
        if (m_preview->contentWidget()) {
            m_preview->contentWidget()->setParent(nullptr);
            m_preview->contentWidget()->hide();
            m_vMainLayout->removeWidget(m_preview->contentWidget());
        }

        if (m_preview->toolBarWidget()) {
            m_preview->toolBarWidget()->setParent(nullptr);
            m_preview->toolBarWidget()->hide();
            m_vMainLayout->removeWidget(m_preview->toolBarWidget());
        }
    }

    m_vMainLayout->removeWidget(m_detailInfoWidget);
    m_vMainLayout->removeWidget(m_aiToolBar);
    m_vMainLayout->removeItem(m_vSpaceItem);

    if (m_generalPreview) {
        if (m_generalPreview->contentWidget()) {
            m_generalPreview->contentWidget()->setParent(nullptr);
            m_generalPreview->contentWidget()->hide();
            m_vMainLayout->removeWidget(m_generalPreview->contentWidget());
        }
        if (m_generalPreview->toolBarWidget()) {
            m_generalPreview->toolBarWidget()->setParent(nullptr);
            m_generalPreview->toolBarWidget()->hide();
            m_vMainLayout->removeWidget(m_generalPreview->toolBarWidget());
        }
    }
}

int PreviewWidget::calculateContentHeight(bool hasDetailInfo, bool showAiToolBar) const
{
    int totalHeight = this->height();
    if (totalHeight <= 0)
        return 0;

    int reservedHeight = 0;

    // 工具栏（通用工具栏或自定义工具栏）
    if (m_preview && m_preview->showToolBar()) {
        QWidget *toolBar = m_preview->toolBarWidget();
        if (!toolBar)
            toolBar = m_generalToolBar;
        reservedHeight += toolBar->sizeHint().height();
    }

    // AI工具栏
    if (showAiToolBar) {
        reservedHeight += m_aiToolBar->sizeHint().height();
    }

    // 属性详情部件
    if (hasDetailInfo) {
        reservedHeight += m_detailInfoWidget->sizeHint().height();
    }

    return qMax(0, totalHeight - reservedHeight);
}

void PreviewWidget::onOpenClicked()
{
    qCDebug(logGrandSearch) << "Preview open clicked:" << m_item.name;
    Utils::openMatchedItem(m_item);
}

void PreviewWidget::onOpenpathClicked()
{
    qCDebug(logGrandSearch) << "Preview open path clicked:" << m_item.item;
    Utils::openInFileManager(m_item);
}

void PreviewWidget::onCopypathClicked()
{
    qCDebug(logGrandSearch) << "Preview copy path clicked:" << m_item.item;
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_item.item);
}

void PreviewWidget::updateHighlightContent(const QString &filePath, const QString &content)
{
    // 仅当文件路径匹配当前预览项且内容非空时更新
    if (filePath.isEmpty() || content.isEmpty()
        || m_item.item != filePath || this->isHidden()) {
        return;
    }

    qCDebug(logGrandSearch) << "Updating preview highlight for:" << m_item.name;

    // 更新 m_item 中的 matchedContext
    QVariantHash extraHash = m_item.extra.toHash();
    extraHash.insert(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT, content);
    m_item.extra = extraHash;

    // 直接更新当前预览插件的内容标签，避免完整布局重建
    if (m_preview) {
        ItemInfo itemInfo;
        itemInfo[PREVIEW_ITEMINFO_ITEM] = m_item.item;
        itemInfo[PREVIEW_ITEMINFO_NAME] = m_item.name;
        itemInfo[PREVIEW_ITEMINFO_ICON] = m_item.icon;
        itemInfo[PREVIEW_ITEMINFO_TYPE] = m_item.type;
        itemInfo[PREVIEW_ITEMINFO_SEARCHER] = m_item.searcher;
        itemInfo[PREVIEW_ITEMINFO_MATCHEDCONTEXT] = content;

        if (extraHash.contains(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS)) {
            itemInfo[PREVIEW_ITEMINFO_KEYWORDS] = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS).toStringList().join(":");
        }

        m_preview->previewItem(itemInfo);
    }
}
