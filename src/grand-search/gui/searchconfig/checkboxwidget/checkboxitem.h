// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXITEM_H
#define CHECKBOXITEM_H

#include <DWidget>
#include <DCheckBox>

class CheckBoxItem : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit CheckBoxItem(const QString &text, QWidget *parent = nullptr);
    ~CheckBoxItem();

    void setChecked(bool checked);
    void setTopRound(bool round);
    void setBottomRound(bool round);
    QString text() const;

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Dtk::Widget::DCheckBox *m_checkBox = nullptr;
    bool m_topRound = false;
    bool m_bottomRound = false;
};

#endif   // CHECKBOXITEM_H
