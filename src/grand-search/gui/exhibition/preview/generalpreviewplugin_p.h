/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
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
#ifndef GENERALPREVIEWPLUGIN_P_H
#define GENERALPREVIEWPLUGIN_P_H

#include "generalpreviewplugin.h"

#include <DHorizontalLine>

#include <QLabel>
#include <QVBoxLayout>

class NameLabel: public QLabel
{
    Q_OBJECT

public:
    explicit NameLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SectionKeyLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionKeyLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SectionValueLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionValueLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class ActionLabel: public QLabel
{
    Q_OBJECT

public:
    explicit ActionLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});

signals:
    void sigLabelClicked();

private:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QColor m_textColor;
    QColor m_textColor_clicked;
};

//自定义水平布局 支持显示/隐藏指定行
class QHBoxLayout;
class VBoxLayoutEx : public QVBoxLayout
{
    Q_OBJECT

public:
    explicit VBoxLayoutEx(QWidget *parent = nullptr);
    ~VBoxLayoutEx();

    void addRow(QWidget *label, QWidget *field);
    QWidget* getRowLabel(int nRow);
    QWidget* getRowField(int nRow);
    QHBoxLayout* getRowLayout(int nRow);
    void showRow(int nRow, bool bShow);

private:
    QList<QHBoxLayout*> m_vHLayout;
    QList<QWidget*> m_vLabels;
    QList<QWidget*> m_vFields;
};

DWIDGET_BEGIN_NAMESPACE
class DHorizontalLine;
DWIDGET_END_NAMESPACE
class QVBoxLayout;
class GeneralPreviewPluginPrivate
{
public:
    enum BasicInfoRow {
        Location_Row,
        Size_Row,
        ContainSize_Row,
        TimeModified_Row,
        RowCount
    };

    explicit GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent = nullptr);

    QString getBasicInfoLabelName(BasicInfoRow eRow);
    void setBasicInfo(const GrandSearch::MatchedItem &item);

    GeneralPreviewPlugin *q_p = nullptr;

    GrandSearch::MatchedItem m_item;

    QVBoxLayout *m_vMainLayout = nullptr;
    QPointer<QWidget> m_contentWidget = nullptr;

    // 图标和名称
    QLabel *m_iconLabel = nullptr;
    NameLabel *m_nameLabel = nullptr;

    // 分割线1
    Dtk::Widget::DHorizontalLine *m_line1 = nullptr;

    // 基本信息
    VBoxLayoutEx* m_basicInfoLayout = nullptr;

    // 分割线2
    Dtk::Widget::DHorizontalLine *m_line2 = nullptr;

    // 动作标签 打开、打开路径、复制路径
    ActionLabel* m_openLabel = nullptr;
    ActionLabel* m_openPathLabel = nullptr;
    ActionLabel* m_copyPathLabel = nullptr;
};

#endif // UNKNOWNPREVIEWPLUGIN_P_H
