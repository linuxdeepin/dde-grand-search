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
#ifndef GRANDSEARCHWIDGET_H
#define GRANDSEARCHWIDGET_H

#include <DGuiApplicationHelper>

#include <QWidget>
#include <QIcon>
#include <QPixmap>

namespace GrandSearch {

class GrandSearchInterface;

class GrandSearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GrandSearchWidget(QWidget *parent = nullptr);
    ~GrandSearchWidget() override;

    QString itemCommand(const QString &itemKey);
    QPixmap iconPixmap(QSize iconSize, DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType themeType) const;

private slots:
    void grandSearchVisibleChanged(bool visible);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    const QPixmap loadSvg(const QString &fileName, const QSize &size) const;

private:
    bool m_hover = false;       // 鼠标是否悬浮
    bool m_pressed = false;     // 鼠标是否按下
    QIcon m_icon;
    QPixmap m_pixmap;
    bool m_grandSearchVisible = false;
    GrandSearchInterface *m_grandSearchInterface = nullptr;
};

}

#endif // GRANDSEARCHWIDGET_H
