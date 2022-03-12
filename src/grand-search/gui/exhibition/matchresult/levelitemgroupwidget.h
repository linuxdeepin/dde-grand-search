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
#ifndef LEVELITEMGROUPWIDGET_H
#define LEVELITEMGROUPWIDGET_H

#include "groupwidget.h"

class LevelItemGroupWidget : public GroupWidget
{
public:
    explicit LevelItemGroupWidget(QWidget *parent = nullptr);

    void appendMatchedItems(const GrandSearch::MatchedItems &newItems, const QString& searchGroupName) override;
    void clear() override;

public slots:
    void onMoreBtnClicked() override;

private:
    void takeItemFromLevelCache();
    void takeItemFromGeneralCache();
    void splitLevelData(const GrandSearch::MatchedItems &newItems, QMap<int, GrandSearch::MatchedItems> &newLevelItems, GrandSearch::MatchedItems &newGeneralItems);
private:
    QMap<int, GrandSearch::MatchedItems> m_levelCacheItems;     //<层级, 该层的缓解项>
};

#endif // LEVELITEMGROUPWIDGET_H
