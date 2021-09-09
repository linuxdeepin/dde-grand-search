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
#ifndef DETAILINFOWIDGET_H
#define DETAILINFOWIDGET_H

#include "previewplugin.h"

#include <DHorizontalLine>
#include <DWidget>

#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QToolButton>

class SectionKeyLabel: public QWidget
{
    Q_OBJECT

public:
    explicit SectionKeyLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});

    void setText(const QString &text);
    QLabel *label();
private:
    QLabel* m_label;
};

class SectionValueLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionValueLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

//自定义表单布局 支持显示/隐藏指定行
class FormLayoutEx : public QFormLayout
{
    Q_OBJECT

public:
    explicit FormLayoutEx(QWidget *parent = nullptr);
    ~FormLayoutEx();

    void addRow(QWidget *label, QWidget *field);
    QWidget* getRowLabel(int nRow);
    QWidget* getRowField(int nRow);
    void showRow(int nRow, bool bShow);
    void setRowHeight(int nHeight);
    int rowCount();
    QRect getRowRect(int nRow);
    void clear();

private:
    QList<QWidget*> m_vLabels;
    QList<QWidget*> m_vFields;
};

class DetailInfoWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit DetailInfoWidget(QWidget* parent = nullptr);
    ~DetailInfoWidget();

    void setDetailInfo(const DetailInfoList &infos);
    void setRowHeight(int nHeight);

private:
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    FormLayoutEx* m_formLayout;

    DetailInfoList m_vInfos;

    int m_nRowHeight;
};

#endif // DETAILINFOWIDGET_H
