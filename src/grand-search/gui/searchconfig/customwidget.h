/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <DWidget>
#include <QVBoxLayout>
#include <QLabel>

namespace GrandSearch {

class PlanWidget;
class BestMatchWidget;
class TailerWidget;
class SearchEngineWidget;
class SwitchWidget;
class CustomWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit CustomWidget(QWidget *parent = nullptr);
    ~CustomWidget();

private slots:
    void onSwitchStateChanged(const bool checked);

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    PlanWidget *m_searchPlanWidget = nullptr;
    TailerWidget *m_tailerWidget = nullptr;
    BestMatchWidget *m_bestMatchWidget = nullptr;
    SearchEngineWidget *m_searchEngineWidget = nullptr;
    QVBoxLayout *m_innerLayout = nullptr;

    QHash<QString, QString> m_groupName;        // <searchGroupName, displayGroupName>
    QList<SwitchWidget*> m_switchWidgets;

    QStringList m_displayIcons;                 // 显示图标
};

}

#endif // CUSTOMWIDGET_H
