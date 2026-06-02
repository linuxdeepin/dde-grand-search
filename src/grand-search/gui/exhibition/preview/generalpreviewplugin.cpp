// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generalpreviewplugin_p.h"
#include "generalpreviewplugin.h"
#include "utils/utils.h"
#include "global/commontools.h"
#include "global/widgets/highlightlabel.h"

#include <DFontSizeManager>
#include <DMessageBox>
#include <DGuiApplicationHelper>

#include <QIcon>
#include <QGuiApplication>
#include <QClipboard>
#include <QToolButton>
#include <QPainter>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

#define ICON_SIZE               96
#define HOR_MARGIN_SIZE         10
#define MARGIN_SIZE             15
#define NAME_WIDTH              239

using namespace GrandSearch;

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

SizeLabel::SizeLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SizeLabel");
    setFixedWidth(NAME_WIDTH);
    QColor textColor(0, 0, 0, int(255 * 0.4));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, int(255 * 0.4));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

GeneralPreviewPluginPrivate::GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent)
    : q_p(parent)
    , m_contentWidget(new QWidget())
{
    // 图标和名称
    m_iconLabel = new QLabel(m_contentWidget);
    m_iconLabel->setObjectName("IconLabel");
    m_iconLabel->setFixedSize(QSize(ICON_SIZE, ICON_SIZE));

    // 名称标签：中间省略，最多2行
    m_nameLabel = new HighlightLabel(m_contentWidget);
    m_nameLabel->setObjectName("NameLabel");
    m_nameLabel->setFixedWidth(NAME_WIDTH);
    QFont titleFont = DFontSizeManager::instance()->get(DFontSizeManager::T5, QFont::Normal);
    m_nameLabel->setFont(titleFont);
    QPalette pa = m_nameLabel->palette();
    QColor textColor = pa.brightText().color();
    textColor.setAlpha(255 * 0.9);
    pa.setColor(QPalette::WindowText, textColor);
    m_nameLabel->setPalette(pa);
    m_nameLabel->setElideMode(HighlightLabel::ElideMode::Middle);
    m_nameLabel->setMaxLines(2);

    // 内容标签：右省略，最多3行
    m_contentLabel = new HighlightLabel(m_contentWidget);
    pa = m_contentLabel->palette();
    textColor = pa.brightText().color();
    textColor.setAlpha(255 * 0.5);
    pa.setColor(QPalette::WindowText, textColor);
    QFont contentFont = DFontSizeManager::instance()->get(DFontSizeManager::T7, QFont::Normal);
    m_contentLabel->setFont(contentFont);
    m_contentLabel->setPalette(pa);
    m_contentLabel->setFixedWidth(NAME_WIDTH);
    m_contentLabel->setElideMode(HighlightLabel::ElideMode::Right);
    m_contentLabel->setMaxLines(3);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vLayout->addWidget(m_nameLabel);
    vLayout->addWidget(m_contentLabel);
    vLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(HOR_MARGIN_SIZE - 3, MARGIN_SIZE, HOR_MARGIN_SIZE, MARGIN_SIZE);
    hLayout->setSpacing(HOR_MARGIN_SIZE);
    hLayout->addWidget(m_iconLabel);
    hLayout->addLayout(vLayout);

    m_vMainLayout = new QVBoxLayout(m_contentWidget);

    m_vMainLayout->setContentsMargins(MARGIN_SIZE, 0, MARGIN_SIZE, MARGIN_SIZE);
    m_vMainLayout->addLayout(hLayout);
}

GeneralPreviewPluginPrivate::~GeneralPreviewPluginPrivate()
{
    m_contentWidget->deleteLater();
}

GeneralPreviewPlugin::GeneralPreviewPlugin(QObject *parent)
    : QObject(parent)
    , PreviewPlugin()
    , d_p(new GeneralPreviewPluginPrivate(this))
{
    qCDebug(logGrandSearch) << "Creating GeneralPreviewPlugin";
}

GeneralPreviewPlugin::~GeneralPreviewPlugin()
{
    qCDebug(logGrandSearch) << "Destroying GeneralPreviewPlugin";

    if (d_p->m_sizeWorker) {
        d_p->m_sizeWorker->stop();
        d_p->m_sizeWorker->wait();
        d_p->m_sizeWorker->deleteLater();
    }
}

void GeneralPreviewPlugin::init(QObject *proxyInter)
{
    d_p->m_proxy = proxyInter;
}

bool GeneralPreviewPlugin::previewItem(const ItemInfo &info)
{
    MatchedItem item;
    item.item = info[PREVIEW_ITEMINFO_ITEM];
    item.name = info[PREVIEW_ITEMINFO_NAME];
    item.icon = info[PREVIEW_ITEMINFO_ICON];
    item.type = info[PREVIEW_ITEMINFO_TYPE];
    item.searcher = info[PREVIEW_ITEMINFO_SEARCHER];

    // Get matched context if available
    QString matchedContext = info.value(PREVIEW_ITEMINFO_MATCHEDCONTEXT);
    // Get keywords for highlighting
    QStringList keywords;
    QString keywordsStr = info.value(PREVIEW_ITEMINFO_KEYWORDS);
    if (!keywordsStr.isEmpty()) {
        keywords = keywordsStr.split(":", Qt::SkipEmptyParts);
    }

    if (!keywords.isEmpty()) {
        QVariantHash extraHash { { GRANDSEARCH_PROPERTY_ITEM_KEYWORDS, keywords } };
        if (!matchedContext.isEmpty()) {
            extraHash.insert(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT, matchedContext);
        }
        item.extra = extraHash;
    }

    qCDebug(logGrandSearch) << "General preview item - Name:" << item.name
                            << "Type:" << item.type
                            << "Has matched context:" << !matchedContext.isEmpty()
                            << "Keywords count:" << keywords.size();

    if (!item.item.isEmpty()
            && !item.name.isEmpty()
            && d_p->m_item.item == item.item
            && d_p->m_item.name == item.name) {
        const auto &extra = d_p->m_item.extra.toHash();
        const auto &itemKeywords = extra.value(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS, QStringList()).toStringList();
        const auto &oldMatchedContext = extra.value(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT).toString();

        if (!keywords.isEmpty() && keywords == itemKeywords
                && matchedContext == oldMatchedContext) {
            qCDebug(logGrandSearch) << "Item already previewed - skipping";
            return true;
        }
    }

    d_p->m_item = item;

    // 设置图标
    QPixmap pixmap;
    const QSize iconSize(ICON_SIZE, ICON_SIZE);
    const QString &strIcon = item.icon;
    if (!strIcon.isEmpty()) {
        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                pixmap = QIcon(strIcon).pixmap(iconSize);
            else
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (icon.isNull())
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
            else
                pixmap = icon.pixmap(iconSize);
        }
    } else {
        pixmap = Utils::defaultIcon(item).pixmap(iconSize);
    }

    if (pixmap.isNull())
        pixmap = QIcon::fromTheme("unknown").pixmap(iconSize);
    d_p->m_iconLabel->setPixmap(pixmap);

    // 设置名称，关键词高亮在 HighlightLabel 内部处理
    d_p->m_nameLabel->setPlainText(item.name);
    d_p->m_nameLabel->setKeywords(keywords);
    if (d_p->m_nameLabel->isElided())
        d_p->m_nameLabel->setToolTip(item.name);
    else
        d_p->m_nameLabel->setToolTip("");

    // 设置匹配内容，关键词高亮在 HighlightLabel 内部处理
    if (!matchedContext.isEmpty()) {
        d_p->m_contentLabel->setPlainText(matchedContext);
        d_p->m_contentLabel->setKeywords(keywords);
        if (d_p->m_contentLabel->isElided())
            d_p->m_contentLabel->setToolTip(matchedContext);
        else
            d_p->m_contentLabel->setToolTip("");
    } else {
        d_p->m_contentLabel->setPlainText("");
        d_p->m_contentLabel->setKeywords(QStringList());
        d_p->m_contentLabel->setToolTip("");
    }
    QFileInfo fi(item.item);

    // 设置属性详情信息
    d_p->m_detailInfos.clear();

    // 位置信息
    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Location:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fi.absoluteFilePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    d_p->m_detailInfos.push_back(detailInfo);

    // 文件大小
    QString fileSizeStr;
    // 获取文件(夹)大小
    if (fi.isDir()) {
        if (!d_p->m_sizeWorker) {
            d_p->m_sizeWorker = new FileStatisticsThread(this);
            connect(d_p->m_sizeWorker, &FileStatisticsThread::dataNotify, this, &GeneralPreviewPlugin::updateFolderSize);
        } else if (d_p->m_sizeWorker->isRunning()) {
            d_p->m_sizeWorker->stop();
            d_p->m_sizeWorker->wait();
        }

        fileSizeStr = CommonTools::formatFileSize(0);
        d_p->m_sizeWorker->start(item.item);
    } else {
        fileSizeStr = CommonTools::formatFileSize(fi.size());
    }

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Size:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileSizeStr));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    d_p->m_detailInfos.push_back(detailInfo);

    // 修改时间
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Time modified:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(CommonTools::getFileModifiedTime(item.item).toString(CommonTools::dateTimeFormat())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    d_p->m_detailInfos.push_back(detailInfo);

    return true;
}

ItemInfo GeneralPreviewPlugin::item() const
{
    ItemInfo itemInfo;
    itemInfo[PREVIEW_ITEMINFO_ITEM] = d_p->m_item.item;
    itemInfo[PREVIEW_ITEMINFO_NAME] = d_p->m_item.name;
    itemInfo[PREVIEW_ITEMINFO_ICON] = d_p->m_item.icon;
    itemInfo[PREVIEW_ITEMINFO_TYPE] = d_p->m_item.type;
    itemInfo[PREVIEW_ITEMINFO_SEARCHER] = d_p->m_item.searcher;

    return itemInfo;
}

bool GeneralPreviewPlugin::stopPreview()
{
    return true;
}

QWidget *GeneralPreviewPlugin::contentWidget() const
{
    return d_p->m_contentWidget;
}

DetailInfoList GeneralPreviewPlugin::getAttributeDetailInfo() const
{
    return d_p->m_detailInfos;
}

QWidget *GeneralPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool GeneralPreviewPlugin::showToolBar() const
{
    return true;
}

void GeneralPreviewPlugin::updateFolderSize(qint64 size)
{
    // Find and update the size entry in detail infos
    for (int i = 0; i < d_p->m_detailInfos.size(); ++i) {
        DetailTagInfo &tag = d_p->m_detailInfos[i].first;
        if (tag.value(DetailInfoProperty::Text).toString() == tr("Size:")) {
            d_p->m_detailInfos[i].second.insert(DetailInfoProperty::Text, QVariant(CommonTools::formatFileSize(size)));
            break;
        }
    }

    QMetaObject::invokeMethod(d_p->m_proxy, "updateDetailInfo", Qt::AutoConnection, Q_ARG(GrandSearch::PreviewPlugin*, this));
}
