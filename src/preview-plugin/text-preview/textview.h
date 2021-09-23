/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QWidget>

#include <QPlainTextEdit>

class QLabel;
class QStackedWidget;
class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    using QPlainTextEdit::QPlainTextEdit;
protected:
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
};

class TextView : public QWidget
{
    Q_OBJECT
public:
    explicit TextView(QWidget *parent = nullptr);
    void initUI();
    void setSource(const QString &path);
    static QString toUnicode(const QByteArray &data);
    void showErrorPage();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
     PlainTextEdit* m_browser = nullptr;
     QLabel *m_errLabel = nullptr;
     QStackedWidget *m_stackedWidget = nullptr;
};

#endif // TEXTVIEW_H
