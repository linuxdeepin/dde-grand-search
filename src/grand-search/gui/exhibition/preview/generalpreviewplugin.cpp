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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QIcon>

#define ICON_SIZE 24 * 5

using namespace GrandSearch;

GeneralPreviewPluginPrivate::GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent)
    : q_p(parent)
    , m_contentWidget(new QWidget())
{
    m_contentWidget->setFixedSize(350, 520);
    m_iconLabel = new QLabel(m_contentWidget);
    m_iconLabel->setObjectName("IconLabel");
    m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nameLabel = new QLabel(m_contentWidget);
    m_nameLabel->setObjectName("NameLabel");
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setWordWrap(true);
    m_sizeLabel = new QLabel(m_contentWidget);
    m_sizeLabel->setObjectName("SizeLabel");
    m_sizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_typeLabel = new QLabel(m_contentWidget);
    m_typeLabel->setObjectName("TypeLabel");
    m_typeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QVBoxLayout *vlayout = new QVBoxLayout();

    vlayout->addWidget(m_nameLabel);
    vlayout->addWidget(m_sizeLabel);
    vlayout->addWidget(m_typeLabel);
    vlayout->addStretch();

    m_hMainLayout = new QHBoxLayout(m_contentWidget);

    m_hMainLayout->setContentsMargins(0, 0, 0, 0);
    m_hMainLayout->addWidget(m_iconLabel);
    m_hMainLayout->addSpacing(10);
    m_hMainLayout->addLayout(vlayout);
    m_hMainLayout->addStretch();
}

GeneralPreviewPlugin::GeneralPreviewPlugin(QObject *parent)
    : PreviewPlugin(parent)
    , d_p(new GeneralPreviewPluginPrivate(this))
{

}

GeneralPreviewPlugin::~GeneralPreviewPlugin()
{
    Q_ASSERT(d_p->m_contentWidget);
    d_p->m_contentWidget->deleteLater();
}

bool GeneralPreviewPlugin::previewItem(const MatchedItem &item)
{
    if (!item.item.isEmpty()
            && !item.name.isEmpty()
            && d_p->m_item.item == item.item
            && d_p->m_item.name == item.name)
        return true;

    d_p->m_item = item;

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

    QFont font = d_p->m_nameLabel->font();
    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(item.name, Qt::ElideMiddle, 300);

    d_p->m_nameLabel->setText(elidedText);

    if (QFile::exists(item.item)) {
        QFileInfo info(item.item);
        if (info.isFile() || info.isSymLink()) {
            d_p->m_sizeLabel->setText(QObject::tr("Size: %1").arg(info.size()));
            d_p->m_typeLabel->setText(QObject::tr("Type: %1").arg(item.type));
        } else if (info.isDir()) {
//            if (!m_sizeWorker) {
//                m_sizeWorker = new DFileStatisticsJob(this);

//                connect(m_sizeWorker, &DFileStatisticsJob::dataNotify, this, &UnknowFilePreview::updateFolderSize);
//            } else if (m_sizeWorker->isRunning()) {
//                m_sizeWorker->stop();
//                m_sizeWorker->wait();
//            }

//            m_sizeWorker->start({info->fileUrl()});
            d_p->m_sizeLabel->setText(QObject::tr("Size: 0"));
        }
    }

    return true;
}

MatchedItem GeneralPreviewPlugin::item() const
{
    return d_p->m_item;
}

QWidget *GeneralPreviewPlugin::contentWidget() const
{
    return d_p->m_contentWidget;
}
