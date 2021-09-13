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
#include "pdfview.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QPainter>
#include <QLabel>

#define PAGE_MAX_HEIGHT 408

PDFView::PDFView(const QString &file, QWidget *parent)
    :QWidget (parent)
{
    initDoc(file);
    initUI();
    initConnections();
}

PDFView::~PDFView()
{
    if (!m_isLoadFinished)
        m_future.waitForFinished();
}

void PDFView::initDoc(const QString &file)
{
    m_doc = QSharedPointer<Poppler::Document>(Poppler::Document::load(file));
    if (!m_doc || m_doc->isLocked()) {
        qWarning() << "Cannot read this pdf file: " << file;
        m_isBadDoc = true;
    }
}

void PDFView::initUI()
{
    if (m_isBadDoc) {
        showFailedPage();
        return;
    }

    setContentsMargins(0, 0, 0, 0);
    m_pageLabel = new QLabel(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_pageLabel);

    setLayout(layout);
    syncLoadFirstPage();
}

void PDFView::initConnections()
{
    connect(this, &PDFView::pageUpdate, this, &PDFView::onPageUpdated);
}

void PDFView::showFailedPage()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *failedLabel = new QLabel(this);
    failedLabel->setText(tr("Cannot preview this file!"));

    layout->addStretch();
    layout->addWidget(failedLabel, 0, Qt::AlignHCenter);
    layout->addStretch();
    setLayout(layout);
}

void PDFView::onPageUpdated(QImage img)
{
    // 缩放
    img = img.scaledToWidth(m_pageLabel->width(), Qt::SmoothTransformation);
    QImage page(m_pageLabel->width(), std::min(img.height(), PAGE_MAX_HEIGHT), QImage::Format_ARGB32_Premultiplied);
    page.fill(Qt::white);
    QPainter p(&page);
    p.drawImage(0, 0, img);

    m_pageLabel->setPixmap(QPixmap::fromImage(page));
}

void PDFView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (m_pageLabel && !m_pageImg.isNull())
        onPageUpdated(m_pageImg);
}

void PDFView::syncLoadFirstPage()
{
    m_future = QtConcurrent::run([=]{
       QSharedPointer<Poppler::Page> page = QSharedPointer<Poppler::Page>(m_doc->page(0));
       if (!page)
           return;

       // 渲染抗锯齿
       m_doc->setRenderHint(Poppler::Document::Antialiasing, true);
       m_doc->setRenderHint(Poppler::Document::TextAntialiasing, true);
       QImage img = page->renderToImage(200, 200);

       m_pageImg = img;
       emit pageUpdate(img);
       m_isLoadFinished = true;
    });
}
