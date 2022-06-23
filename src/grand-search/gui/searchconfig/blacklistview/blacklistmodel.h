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
#ifndef BLACKLISTMODEL_H
#define BLACKLISTMODEL_H

#include <QStandardItemModel>

#define DATAROLE Qt::UserRole+1

class BlackListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit BlackListModel(int rows, int columns, QObject *parent = Q_NULLPTR);
    ~BlackListModel() override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;

};

#endif // BLACKLISTMODEL_H
