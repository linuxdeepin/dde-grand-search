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

#ifndef BLACKLISTWIDGET_H
#define BLACKLISTWIDGET_H

#include <DWidget>
#include <DIconButton>

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QItemSelection>

DWIDGET_USE_NAMESPACE

class BlackListWrapper;
class DeleteDialog;
class BlackListWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit BlackListWidget(QWidget *parent = nullptr);
    ~BlackListWidget();

private slots:
    void addButtonClicked();
    void deleteButtonClicked();
    void selectedChanged(const QItemSelection &selected, const QItemSelection &deselected) const;

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_childHLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    QLabel *m_contentLabel = nullptr;

    DIconButton *m_addButton = nullptr;
    DIconButton *m_deleteButton = nullptr;

    BlackListWrapper *m_listWrapper = nullptr;
};

#endif // BLACKLISTWIDGET_H
