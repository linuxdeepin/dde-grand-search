/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "../../datadefine.h"
#include "../../../../global/matcheditem.h"

#include <DWidget>

#include <QScopedPointer>


class GroupWidgetPrivate;
class GrandSearchListview;

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DHorizontalLine;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

//#define SHOW_BACKCOLOR

class QVBoxLayout;
class GroupWidget : public DWidget
{
    Q_OBJECT
public:
    explicit GroupWidget(QWidget *parent = nullptr);
    ~GroupWidget();

    void setGroupName(const QString &groupHash);
    void setMatchedItems(const GrandSearch::MatchedItems &items);
    void showHorLine(bool bShow = true);
    bool isHorLineVisilbe();
    GrandSearchListview *getListView();
    int itemCount();
    void reLayout();

    QString groupName();
    static QString getGroupName(const QString &groupHash);
    static QString getGroupObjName(const QString &groupHash);

protected:
    void initUi();
    void initConnect();

    void paintEvent(QPaintEvent *event) override;

public slots:
    void setThemeType(int type);

private:
    QScopedPointer<GroupWidgetPrivate> d_p;

    QVBoxLayout *m_vLayout = nullptr;

    DLabel *m_GroupLabel = nullptr;
    GrandSearchListview *m_ListView = nullptr;
    DHorizontalLine *m_Line = nullptr;

};

typedef QMap<QString, GroupWidget *> GroupWidgetMap;
typedef QList<GroupWidget *> GroupWidgets;

#endif // GROUPWIDGET_H
