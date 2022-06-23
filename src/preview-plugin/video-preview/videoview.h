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
#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <DLabel>

#include <QWidget>

namespace GrandSearch {
namespace video_preview {

class ThumbnailLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ThumbnailLabel(QWidget* parent = nullptr);
    void setPixmap(const QPixmap &pixmap);
private:
    QPixmap m_pixmap;
    QLabel *m_picLabel = nullptr;
};

class VideoView : public QWidget
{
    Q_OBJECT
public:
    explicit VideoView(QWidget *parent = nullptr);
    void initUI();
    void setTitle(const QString &title);
    void setThumbnail(const QPixmap &pixmap);
    static QSize maxThumbnailSize();
signals:

public slots:
protected:
    Dtk::Widget::DLabel *m_title = nullptr;
    ThumbnailLabel *m_picFrame= nullptr;
};

}}

#endif // VIDEOVIEW_H
