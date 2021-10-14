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
#ifndef DETAILITEM_H
#define DETAILITEM_H

#include "../previewplugin.h"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QColor>

#include <DLabel>

class ReplicableLabel;

class DetailItem : public QWidget
{
    Q_OBJECT
public:
    explicit DetailItem(QWidget *parent = nullptr);
    ~DetailItem();

    void setDetailInfo(const GrandSearch::DetailInfo &info);

    void setTopRound(bool round);
    bool topRound() const;

    void setBottomRound(bool round);
    bool bottomRound() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setRadius(int radius);
    int radius() const;

    void setTagWidth(int width);
    int tagWidth() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor getTagColor();
    QColor getContentColor();
protected:
    bool m_topRound = false;
    bool m_bottomRound = false;
    QColor m_backgroundColor;
    int m_radius = 0;

    Dtk::Widget::DLabel *m_tagLabel = nullptr;
    ReplicableLabel *m_contentLabel = nullptr;

    QHBoxLayout *m_mainLayout = nullptr;

};

#endif // DETAILITEM_H
