// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include "gui/iconbutton.h"

#include <QLineEdit>
#include <QScopedPointer>

class QLineEdit;
class QIcon;

namespace GrandSearch {

class SearchEditPrivate;
class SearchEdit : public QWidget
{
    Q_OBJECT
public:
    explicit SearchEdit(QWidget *parent = nullptr);
    ~SearchEdit() override;

    QLineEdit *lineEdit() const;

    void setPlaceHolder(const QString &text);
    QString placeHolder() const;

    void setPlaceholderText(const QString &text);
    QString placeholderText() const;

    void setText(const QString &text);
    QString text() const;

    void setClearButtonEnabled(bool enable);

    void setAppIcon(const QString &iconName);
    void setAppIcon(const QIcon &icon);
    void setAppIconVisible(bool visible);
    bool isAppIconVisible() const;

    void clear();
    void clearEdit();

    void setFocus();

signals:
    void debouncedTextChanged(const QString &text);

    void selectPreviousItem();
    void selectNextItem();
    void handleItem();
    void closeWindow();
    void searchAborted();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;

private:
    QScopedPointer<SearchEditPrivate> d;
};

}

#endif // SEARCHEDIT_H
