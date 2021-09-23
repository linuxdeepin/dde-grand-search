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
#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include "../previewplugin.h"

#include <DWidget>

#include <QVBoxLayout>
#include <QScrollArea>

class DetailItem;

class DetailWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit DetailWidget(QWidget *parent = nullptr);

    void setDetailInfoList(const GrandSearch::DetailInfoList &list);

private:
    QList<DetailItem *> m_detailItems;

    QVBoxLayout *m_mainLayout = nullptr;

    bool m_alignment = false;
};

#endif // DETAILWIDGET_H
