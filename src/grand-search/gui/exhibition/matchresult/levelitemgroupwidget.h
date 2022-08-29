// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
