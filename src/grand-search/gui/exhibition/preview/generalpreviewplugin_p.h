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

class QLabel;
class QHBoxLayout;
class GeneralPreviewPluginPrivate
{
public:
    explicit GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent = nullptr);

    GeneralPreviewPlugin *q_p = nullptr;

    GrandSearch::MatchedItem m_item;

    QHBoxLayout *m_hMainLayout = nullptr;
    QPointer<QWidget> m_contentWidget = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_sizeLabel = nullptr;
    QLabel *m_typeLabel = nullptr;
};

#endif // UNKNOWNPREVIEWPLUGIN_P_H
