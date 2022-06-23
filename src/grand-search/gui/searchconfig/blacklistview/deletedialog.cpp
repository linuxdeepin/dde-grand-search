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

#include "deletedialog.h"

#include <QAbstractButton>

DeleteDialog::DeleteDialog(QWidget *parent)
    : DDialog(parent)
{
    const QString content(tr("Do you want to remove the path from the exclusion list?"));
    setTitle(content);
    setIcon(QIcon(QString(":/icons/%1.svg").arg("dde-grand-search-setting")));

    // the cancel button
    {
        int idx = addButton(tr("Cancel", "button"));
        auto btn = getButton(idx);
        Q_ASSERT(btn);
        connect(btn, &QAbstractButton::clicked, this, &QDialog::reject);
    }

    // the confirm button
    {
        auto idx = addButton(tr("Confirm", "button"));
        auto btn = getButton(idx);
        Q_ASSERT(btn);
        connect(btn, &QAbstractButton::clicked, this, &QDialog::accept);
    }
}

DeleteDialog::~DeleteDialog()
{

}
