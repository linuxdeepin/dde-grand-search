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

#ifndef SEARCHENGINEWIDGET_H
#define SEARCHENGINEWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DSwitchButton>

#include <QVBoxLayout>

namespace GrandSearch {
class ComboboxWidget;
}

class SearchEngineWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit SearchEngineWidget(QWidget *parent = nullptr);
    ~SearchEngineWidget();

private slots:
    void checkedChangedIndex(int index);

private:
    int getIndex(const QString text) const;

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel= nullptr;
    QLabel *m_contentLabel = nullptr;

    QHash<QString, QString> m_groupName;        // <searchGroupName, displayGroupName>
    GrandSearch::ComboboxWidget* m_comboboxWidget;
};

#endif // SEARCHENGINEWIDGET_H
