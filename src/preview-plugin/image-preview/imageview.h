/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <DWidget>
#include <QLabel>
#include <QByteArray>
#include <QMovie>
#include <QVBoxLayout>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

class ImageView : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = nullptr);
    ~ImageView() Q_DECL_OVERRIDE;

    QSize sourceSize();
    bool stopPreview();

    void loadImage(const QString &file, const QString &type);
private:
    void initUI();
    void initConnect();

    bool canPreview();

    QPixmap getRoundPixmap(const QPixmap &pixmap);

private slots:
    void onMovieFrameChanged(int frameNumber);
    void showErrorPage();

private:
    QVBoxLayout *m_mainLayout = nullptr;

    QString m_imageFile;                // 资源文件
    QByteArray m_formats;               // 格式
    QImage m_image;                     // 图片
    QMovie *m_movie = nullptr;          // gif图片
    bool m_isMovie = false;             // 是否是gif图片

    QSize m_sourceSize;                 // 资源尺寸
    QLabel *m_imageLabel;               // 显示图片
    Dtk::Widget::DLabel *m_titleLabel;  // 显示名称
};

#endif // IMAGEVIEW_H
