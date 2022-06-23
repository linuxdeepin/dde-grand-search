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

#ifndef BLACKLISTVIEW_H
#define BLACKLISTVIEW_H

#include <DListView>
#include <DWidget>

#include <QDropEvent>
#include <QVBoxLayout>

namespace GrandSearch {

class BlackListModel;
class BlackListDelegate;
class BlackListView : public Dtk::Widget::DListView
{
    Q_OBJECT
public:
    explicit BlackListView(QWidget *parent = Q_NULLPTR);
    ~BlackListView() override;

    bool removeRows(const int &row, const int &count);
    void addRow(const QString &path);

protected:
    void dropEvent(QDropEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

private:
    void setData(const QModelIndex &index, const QString &path);
    void init();
    void insertRow(const QString &path);
    void appendRow(const QString &path);
    void moveRowToLast(const QModelIndex &index);
    void updateEmptyRows();
    void addModel(const QString &path);
    void blackModelChanged() const;
    QModelIndex match(const QString &path) const;

private:
    BlackListModel *m_model = nullptr;
    BlackListDelegate *m_delegate = nullptr;
    QStringList m_paths;
    int m_validRows = 0;
};

class BlackListWrapper : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit BlackListWrapper(QWidget *parent = nullptr);
    ~BlackListWrapper();

    void addRow(const QString &path) const;
    QItemSelectionModel *selectionModel() const;
    void removeRows(int row, int count);
    void clearSelection();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void selectedChanged(const QItemSelection &selected, const QItemSelection &deselected) const;

private:
    QVBoxLayout *m_mainLayout = nullptr;
    BlackListView *m_listView = nullptr;
};

}

#endif // BLACKLISTVIEW_H
