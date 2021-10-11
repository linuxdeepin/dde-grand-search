/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "videoview.h"

#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QFileInfo>
#include <QPainter>
#include <QDebug>

#define CONTENT_WIDTH 310
#define CONTENT_HEIGHT 110

VideoView::VideoView(QWidget *parent) : QWidget(parent)
{

}

void VideoView::initUI()
{
    // 高150
    setFixedHeight(150);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(0);
    // 左边距35，view外有10的上边距，内部只需22-10=12的上边距
    // 右边距35,view外下边距已有15,内部为0
    lay->setContentsMargins(35, 12, 35, 0);

    //现实缩略图
    m_picFrame = new ThumbnailLabel(this);
    m_picFrame->setFixedSize(CONTENT_WIDTH, CONTENT_HEIGHT);
    lay->addWidget(m_picFrame);

    //显示文件名
    m_title = new DLabel(this);
    m_title->setFixedWidth(CONTENT_WIDTH);
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setElideMode(Qt::ElideMiddle);
    m_title->setMargin(0);

    QFont titleFont = m_title->font();
    titleFont.setWeight(QFont::Medium);
    titleFont = DFontSizeManager::instance()->get(DFontSizeManager::T5, titleFont);
    m_title->setFont(titleFont);

    lay->addWidget(m_title);
}

void VideoView::setTitle(const QString &title)
{
    if (m_title) {
        m_title->setText(title);
        QFontMetrics fontMetrics(m_title->font());
        int textWidth = fontMetrics.size(Qt::TextSingleLine, title).width();
        if (textWidth >= m_title->width())
            m_title->setToolTip(title);
    }
}

void VideoView::setThumbnail(const QPixmap &pixmap)
{
    if (m_picFrame)
        m_picFrame->setPixmap(pixmap);
}

QSize VideoView::maxThumbnailSize()
{
    return QSize(CONTENT_WIDTH, CONTENT_HEIGHT);
}

ThumbnailLabel::ThumbnailLabel(QWidget *parent) : QWidget(parent)
{
    //图片，水平居中
    m_picLabel = new QLabel(this);
    m_picLabel->setMargin(0);
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->addWidget(m_picLabel);
    lay->setMargin(0);
}

void ThumbnailLabel::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    m_picLabel->setFixedSize(pixmap.size());
    m_picLabel->setPixmap(pixmap);
}

