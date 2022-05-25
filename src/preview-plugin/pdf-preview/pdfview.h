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
#ifndef PDFVIEW_H
#define PDFVIEW_H

#include <QWidget>
#include <QFuture>

#include <dpdfdoc.h>

class QLabel;
class PDFView : public QWidget
{
    Q_OBJECT
public:
    explicit PDFView(const QString &file, QWidget *parent = nullptr);
    ~PDFView() Q_DECL_OVERRIDE;

    void initDoc(const QString &file);
    void initUI();
    void initConnections();
    QPixmap scaleAndRound(const QImage &img);
public slots:
    void onPageUpdated(QImage img);
    void showErrorPage();
signals:
    void pageUpdate(const QImage &img);
    void parseFailed();

private:
    void syncLoadFirstPage();

private:
    QLabel *m_pageLabel = nullptr;
    bool m_isBadDoc = false;
    bool m_isLoadFinished = false;
    QSharedPointer<DPdfDoc> m_doc = nullptr;
    QFuture<void> m_future;
    QImage m_pageImg;
};

#endif   // PDFVIEW_H
