/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
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
#include "generalpreviewplugin_p.h"
#include "generalpreviewplugin.h"
#include "utils/utils.h"
#include "global/commontools.h"

#include <DFontSizeManager>
#include <DMessageBox>
#include <DGuiApplicationHelper>

#include <QIcon>
#include <QGuiApplication>
#include <QClipboard>
#include <QToolButton>
#include <QPainter>

#define ICON_SIZE               96
#define HOR_MARGIN_SIZE         10
#define MARGIN_SIZE             15
#define NAME_WIDTH              239

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

NameLabel::NameLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("NameLabel");
    setFixedWidth(NAME_WIDTH);

    QColor textColor = QColor(0, 0, 0, static_cast<int>(255 * 0.9));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.9));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    font.setWeight(QFont::Medium);

    setAlignment(Qt::AlignBottom | Qt::AlignLeft);
}

SizeLabel::SizeLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SizeLabel");
    setFixedWidth(NAME_WIDTH);
    QColor textColor = QColor(124, 124, 124, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 1));
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
    m_nameLabel = new NameLabel("", m_contentWidget);
    m_sizeLabel = new SizeLabel("", m_contentWidget);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vLayout->addWidget(m_nameLabel);
    vLayout->addWidget(m_sizeLabel);
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

QString GeneralPreviewPluginPrivate::getBasicInfoLabelName(GeneralPreviewPluginPrivate::BasicInfoRow eRow)
{
    QString name = "";

    switch (eRow) {
    case Location_Row:
        name = QObject::tr("Location:");
        break;
    case TimeModified_Row:
        name = QObject::tr("Time modified:");
        break;
    default:
        break;
    }

    return name;
}

GeneralPreviewPlugin::GeneralPreviewPlugin(QObject *parent)
    : QObject(parent)
    , PreviewPlugin()
    , d_p(new GeneralPreviewPluginPrivate(this))
{

}

GeneralPreviewPlugin::~GeneralPreviewPlugin()
{

}

void GeneralPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
}

bool GeneralPreviewPlugin::previewItem(const GrandSearch::ItemInfo &info)
{
    GrandSearch::MatchedItem item;
    item.item = info[PREVIEW_ITEMINFO_ITEM];
    item.name = info[PREVIEW_ITEMINFO_NAME];
    item.icon = info[PREVIEW_ITEMINFO_ICON];
    item.type = info[PREVIEW_ITEMINFO_TYPE];
    item.searcher = info[PREVIEW_ITEMINFO_SEARCHER];

    if (!item.item.isEmpty()
            && !item.name.isEmpty()
            && d_p->m_item.item == item.item
            && d_p->m_item.name == item.name)
        return true;

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
    d_p->m_iconLabel->setPixmap(pixmap);

    // 设置名称，并计算换行内容
    QString elidedText = lineFeed(item.name, d_p->m_nameLabel->width(), d_p->m_nameLabel->font(), 1);
    d_p->m_nameLabel->setText(elidedText);
    if (elidedText != item.name)
        d_p->m_nameLabel->setToolTip(item.name);

    QFileInfo fi(item.item);

    // 获取文件(夹)大小
    d_p->m_sizeLabel->setText(CommonTools::formatFileSize(fi.size()));

    // 设置属性详情信息
    d_p->m_detailInfos.clear();
    DetailInfo detail = qMakePair(d_p->getBasicInfoLabelName(GeneralPreviewPluginPrivate::Location_Row),
                                fi.absoluteFilePath());
    d_p->m_detailInfos.push_back(detail);

    detail = qMakePair(d_p->getBasicInfoLabelName(GeneralPreviewPluginPrivate::TimeModified_Row),
            fi.lastModified().toString(CommonTools::dateTimeFormat()));
    d_p->m_detailInfos.push_back(detail);

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

QString GeneralPreviewPlugin::lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow)
{
    if (nElidedRow < 0)
        nElidedRow = 2;

    QString strText = text;
    int nIndex = 1;
    QFontMetrics fm(font);
    if (!strText.isEmpty()) {
        for (int i = 1; i < strText.size() + 1; i++) {
            if (fm.width(strText.left(i)) > nWidth * nIndex) {
                nIndex++;
                if (nIndex > nElidedRow) {
                    strText = fm.elidedText(strText, Qt::ElideMiddle, nWidth * nElidedRow);
                    break;
                }
                strText.insert(i - 1, "\n");
            }
        }
    }

    return strText;
}
