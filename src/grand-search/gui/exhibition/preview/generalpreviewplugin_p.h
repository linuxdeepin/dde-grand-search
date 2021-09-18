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
#ifndef GENERALPREVIEWPLUGIN_P_H
#define GENERALPREVIEWPLUGIN_P_H

#include "generalpreviewplugin.h"
#include "detailinfowidget.h"

#include <DHorizontalLine>
#include <DWidget>

#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QToolButton>

class NameLabel: public QLabel
{
    Q_OBJECT
public:
    explicit NameLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SizeLabel: public QLabel
{
    Q_OBJECT
public:
    explicit SizeLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

DWIDGET_BEGIN_NAMESPACE
class DHorizontalLine;
DWIDGET_END_NAMESPACE
class GeneralPreviewPluginPrivate
{
public:
    explicit GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent = nullptr);
    ~GeneralPreviewPluginPrivate();

    GeneralPreviewPlugin *q_p = nullptr;

    GrandSearch::MatchedItem m_item;
    GrandSearch::DetailInfoList m_detailInfos;

    QVBoxLayout *m_vMainLayout = nullptr;
    QPointer<QWidget> m_contentWidget = nullptr;

    // 图标和名称
    QLabel *m_iconLabel = nullptr;
    NameLabel *m_nameLabel = nullptr;

    // 大小
    SizeLabel *m_sizeLabel = nullptr;
};

#endif // UNKNOWNPREVIEWPLUGIN_P_H
