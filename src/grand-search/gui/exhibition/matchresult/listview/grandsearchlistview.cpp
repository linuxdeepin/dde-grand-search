// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchlistview.h"
#include "grandsearchlistmodel.h"
#include "grandsearchlistdelegate.h"
#include "utils/utils.h"
#include "utils/highlightprovider.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "global/accessibility/acintelfunctions.h"
#include "thumbnail/thumbnail.h"

#include <dfm-search/dsearch_global.h>

#include <DGuiApplicationHelper>
#include <DStandardPaths>

#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QMouseEvent>

using namespace GrandSearch;
DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#define ICON_SIZE 24

GrandSearchListView::GrandSearchListView(QWidget *parent)
    : DListView(parent)
{
    m_model = new GrandSearchListModel(0, 0, this);
    setModel(m_model);
    m_delegate = new GrandSearchListDelegate(this);
    setItemDelegate(m_delegate);
    setViewportMargins(0, 0, 0, 0);
    setUniformItemSizes(false);
    setIconSize({ ICON_SIZE, ICON_SIZE });

    setViewMode(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);
    setMouseTracking(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GrandSearchListView::onSetThemeType);

    m_themeType = DGuiApplicationHelper::instance()->themeType();

    // 初始化缩略图模块
    initThumbnailModule();

    // 连接缩略图生成完成信号
    connect(ThumbnailProvider::instance(), &ThumbnailProvider::thumbnailReady,
            this, &GrandSearchListView::onThumbnailReady);
}

GrandSearchListView::~GrandSearchListView()
{
    // 主动释放缩略图任务管理器资源
    ThumbnailTaskManager::instance()->shutdown();
}

void GrandSearchListView::setMatchedItems(const MatchedItems &items)
{
    m_matchedItems = items;
    m_model->clear();
    for (MatchedItem item : m_matchedItems) {
        addRow(item);
    }
}

void GrandSearchListView::addRow(const MatchedItem &item)
{
    QStandardItem *newItem = new QStandardItem;

    AC_SET_ACCESSIBLE_TEXT(newItem, item.name);

    m_model->appendRow(newItem);

    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());

    setData(index, item);
}

void GrandSearchListView::addRow(const MatchedItem &item, const int level)
{
    int lastRow = levelItemLastRow(level);
    insertRow(lastRow, item);
}

void GrandSearchListView::addRows(const MatchedItems &items)
{
    for (auto item : items)
        addRow(item);
}

void GrandSearchListView::addRows(const MatchedItems &items, const int level)
{
    int lastRow = levelItemLastRow(level);
    insertRows(lastRow, items);
}

int GrandSearchListView::insertRow(int nRow, const MatchedItem &item)
{
    if (nRow < 0 || nRow >= m_model->rowCount()) {
        addRow(item);
        return m_model->rowCount() - 1;
    }

    QStandardItem *newItem = new QStandardItem;
    m_model->insertRow(nRow, newItem);

    QModelIndex index = m_model->index(nRow, 0, QModelIndex());
    setData(index, item);

    return nRow;
}

void GrandSearchListView::insertRows(int nRow, const MatchedItems &items)
{
    if (nRow < 0 || nRow >= m_model->rowCount()) {
        addRows(items);
        return;
    }

    for (int i = 0; i < items.size(); i++) {
        insertRow(nRow, items[i]);
    }
}

void GrandSearchListView::removeRows(int nRow, int nCount)
{
    m_model->removeRows(nRow, nCount);
}

int GrandSearchListView::levelItemCount(const int level)
{
    int count = 0;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto item = m_model->data(m_model->index(i, 0, QModelIndex()), DATA_ROLE).value<MatchedItem>();
        // 如果当前项不是分层项，则说明后续已经没有分层项
        int tempLevel = -1;
        if (!Utils::isLevelItem(item, tempLevel))
            break;
        if (-1 == level || level == tempLevel)
            count++;
    }

    return count;
}

int GrandSearchListView::rowCount()
{
    return m_model->rowCount();
}

int GrandSearchListView::getThemeType() const
{
    return m_themeType;
}

void GrandSearchListView::clear()
{
    if (m_delegate)
        m_delegate->clearTooltipCache();
    if (m_model)
        m_model->clear();
}

void GrandSearchListView::setSearchKeyword(const QString &keyword)
{
    // 关键词改变时，丢弃所有旧的高亮任务
    if (!m_currentKeyword.isEmpty()) {
        HighlightProvider::instance()->cancelTask(m_currentKeyword);
    }
    m_currentKeyword = keyword;
}

void GrandSearchListView::updatePreviewItemState(const bool preview)
{
    m_isPreviewItem = preview;
}

bool GrandSearchListView::isPreviewItem() const
{
    return m_isPreviewItem;
}

void GrandSearchListView::onSetThemeType(int type)
{
    m_themeType = type;
}

bool GrandSearchListView::event(QEvent *event)
{
    Q_UNUSED(event)
    return QListView::event(event);
}

void GrandSearchListView::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton: {
        const QModelIndex &index = indexAt(event->pos());
        if (!index.isValid())
            break;

        MatchedItem item = index.data(DATA_ROLE).value<MatchedItem>();
        if (event->modifiers() == Qt::ControlModifier) {
            Utils::openMatchedItemWithCtrl(item);
            emit sigItemClicked();
        } else {
            if (this->currentIndex() != index) {
                setCurrentIndex(index);
                // 通知选择已经改变
                emit sigCurrentItemChanged(item);
            } else {
                Utils::openMatchedItem(item);
                emit sigItemClicked();
            }
        }

        break;
    }
    default:
        break;
    }

    DListView::mousePressEvent(event);
}

void GrandSearchListView::onThumbnailReady(const QString &filePath, const QPixmap &thumbnail)
{
    if (thumbnail.isNull()) {
        return;
    }

    updateThumbnail(filePath, thumbnail);
}

QString GrandSearchListView::cacheDir()
{
    auto userCachePath = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).value(0);
    return userCachePath;
}

void GrandSearchListView::setData(const QModelIndex &index, const MatchedItem &item)
{
    if (!index.isValid())
        return;

    QVariant searchMeta;
    searchMeta.setValue(item);
    m_model->setData(index, searchMeta, DATA_ROLE);

    // 设置icon - 先显示默认图标
    QIcon itemIcon = Utils::defaultIcon(item);
    const QString &strIcon = item.icon;

    if (!strIcon.isEmpty()) {
        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                itemIcon = QIcon(strIcon);
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (!icon.isNull())
                itemIcon = icon;
        }
    }

    if (itemIcon.isNull())
        itemIcon = QIcon::fromTheme("unknown");
    m_model->setData(index, itemIcon, Qt::DecorationRole);

    // 异步请求缩略图
    if (!item.item.isEmpty()) {
        QString mimetype = item.type;
        if (mimetype.isEmpty()) {
            mimetype = Utils::getFileMimetype(item.item);
        }

        // 检查是否支持该类型的缩略图
        if (ThumbnailProvider::instance()->isSupported(mimetype)) {
            ThumbnailProvider::instance()->requestThumbnail(item.item, mimetype, GrandSearch::ThumbnailSize::Large);
        }

        // 异步请求高亮内容（仅对全文搜索和OCR搜索）
        requestHighlightContent(item, true);
    }
}

int GrandSearchListView::levelItemLastRow(const int level)
{
    int lastRow = 0;
    if (level < 1) {
        lastRow = m_model->rowCount();
    } else {
        for (int i = 1; i <= level; ++i) {
            lastRow += levelItemCount(i);
        }
    }

    return lastRow;
}

void GrandSearchListView::updateThumbnail(const QString &filePath, const QPixmap &thumbnail)
{
    if (filePath.isEmpty() || thumbnail.isNull()) {
        return;
    }

    // 遍历所有项，找到匹配的文件路径
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QModelIndex index = m_model->index(row, 0);
        if (!index.isValid()) {
            continue;
        }

        // 获取该项的数据
        QVariant data = m_model->data(index, DATA_ROLE);
        if (!data.isValid()) {
            continue;
        }

        MatchedItem item = data.value<MatchedItem>();
        if (item.item == filePath) {
            // 更新缩略图到独立角色，保留原始图标
            m_model->setData(index, thumbnail, THUMBNAIL_ROLE);
            break;   // 找到后退出循环
        }
    }
}

void GrandSearchListView::requestHighlightContent(const MatchedItem &item, bool highPriority)
{
    // 高亮内容
    static QMap<QString, int> supportHighlightSearchers {
        { GRANDSEARCH_CLASS_FILE_FULLTEXT, DFMSEARCH::SearchType::Content },
        { GRANDSEARCH_CLASS_OCR_TEXT, DFMSEARCH::SearchType::Ocr },
        { GRANDSEARCH_CLASS_FILE_SEMANTIC, DFMSEARCH::SearchType::Semantic }
    };

    if (!supportHighlightSearchers.contains(item.searcher)) {
        return;
    }

    if (m_currentKeyword.isEmpty() || item.item.isEmpty()) {
        return;
    }

    QVariantHash extraHash = item.extra.toHash();

    // 如果已经有 matchedContext，不需要再请求
    if (extraHash.contains(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT)
        && !extraHash.value(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT).toString().isEmpty()) {
        return;
    }

    // 从 item.extra 中获取实际搜索关键词（daemon 可能对用户输入做了转换/扩展）
    QStringList keywords = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS, QStringList()).toStringList();
    if (keywords.isEmpty()) {
        return;
    }

    // 将关键词列表用空格连接为单个字符串，传给 fetchHighlight
    HighlightProvider::instance()->requestHighlight(m_currentKeyword, item.item, keywords.first(),
                                                    supportHighlightSearchers.value(item.searcher), highPriority);
}

void GrandSearchListView::onHighlightReady(const QString &keyword, const QString &filePath, const QString &content)
{
    Q_UNUSED(keyword)
    if (content.isEmpty()) {
        return;
    }

    updateHighlight(filePath, content);
}

void GrandSearchListView::updateHighlight(const QString &filePath, const QString &content)
{
    if (filePath.isEmpty() || content.isEmpty()) {
        return;
    }

    // 遍历所有项，找到匹配的文件路径
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QModelIndex index = m_model->index(row, 0);
        if (!index.isValid()) {
            continue;
        }

        QVariant data = m_model->data(index, DATA_ROLE);
        if (!data.isValid()) {
            continue;
        }

        MatchedItem item = data.value<MatchedItem>();
        if (item.item == filePath) {
            // 更新 extra hash 中的 matchedContext
            QVariantHash extraHash = item.extra.toHash();
            extraHash.insert(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT, content);
            item.extra = extraHash;

            QVariant searchMeta;
            searchMeta.setValue(item);
            m_model->setData(index, searchMeta, DATA_ROLE);
            break;
        }
    }
}
