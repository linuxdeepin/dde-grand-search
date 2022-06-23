/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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
#include "blacklistmodel.h"

using namespace GrandSearch;

BlackListModel::BlackListModel(int rows, int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
{

}

BlackListModel::~BlackListModel()
{

}

Qt::ItemFlags BlackListModel::flags(const QModelIndex &index) const
{
    auto path = index.data(DATAROLE);
    auto flags = QStandardItemModel::flags(index);

    if (path.toString().isEmpty())
        flags &= ~Qt::ItemIsSelectable;
    else
        flags |= Qt::ItemIsSelectable;
    return flags;
}

QStringList BlackListModel::mimeTypes() const
{
    return QStandardItemModel::mimeTypes() << QLatin1String("text/uri-list");
}

