/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "audioview.h"
#include "global/commontools.h"

#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QFileInfo>
#include <QMimeDatabase>

#define ICON_SIZE               96
#define NAME_WIDTH              240
#define HOR_MARGIN_SIZE         10
#define MARGIN_SIZE             15

DWIDGET_USE_NAMESPACE

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
    setFixedHeight(17);
    QColor textColor = QColor(124, 124, 124, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 1));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

AudioView::AudioView(QWidget *parent)
    : QWidget (parent)
{
    initUI();
}

void AudioView::setItemInfo(const GrandSearch::ItemInfo &item)
{
    // 设置图标
    QPixmap pixmap;
    const QSize iconSize(ICON_SIZE, ICON_SIZE);
    const QString &strIcon = item[PREVIEW_ITEMINFO_ICON];
    if (!strIcon.isEmpty()) {
        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                pixmap = QIcon(strIcon).pixmap(iconSize);
            else
                pixmap = defaultIcon(item[PREVIEW_ITEMINFO_ITEM]).pixmap(iconSize);
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (icon.isNull())
                pixmap = defaultIcon(item[PREVIEW_ITEMINFO_ITEM]).pixmap(iconSize);
            else
                pixmap = icon.pixmap(iconSize);
        }
    } else {
        pixmap = defaultIcon(item[PREVIEW_ITEMINFO_ITEM]).pixmap(iconSize);
    }
    m_iconLabel->setPixmap(pixmap);

    // 设置名称，并计算换行内容
    QString elidedText = lineFeed(item[PREVIEW_ITEMINFO_NAME], m_nameLabel->width(), m_nameLabel->font(), 1);
    m_nameLabel->setText(elidedText);
    if (elidedText != item[PREVIEW_ITEMINFO_NAME])
        m_nameLabel->setToolTip(item[PREVIEW_ITEMINFO_NAME]);

    // 获取文件大小
    QFileInfo fi(item[PREVIEW_ITEMINFO_ITEM]);
    m_sizeLabel->setText(GrandSearch::CommonTools::formatFileSize(fi.size()));
}

void AudioView::initUI()
{
    // 图标和名称
    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("IconLabel");
    m_iconLabel->setFixedSize(QSize(ICON_SIZE, ICON_SIZE));
    m_nameLabel = new NameLabel("", this);
    m_sizeLabel = new SizeLabel("", this);

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

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(MARGIN_SIZE, 0, MARGIN_SIZE, MARGIN_SIZE);
    mainLayout->addLayout(hLayout);
}

QString AudioView::lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow)
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
                    strText = fm.elidedText(strText, Qt::ElideRight, nWidth * nElidedRow);
                    break;
                }
                strText.insert(i - 1, "\n");
            }
        }
    }

    return strText;
}

QIcon AudioView::defaultIcon(const QString &fileName)
{
    static QMimeDatabase m_mimeDb;
    return QIcon::fromTheme(m_mimeDb.mimeTypeForFile(fileName).genericIconName());
}
