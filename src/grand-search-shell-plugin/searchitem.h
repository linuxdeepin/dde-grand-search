// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <applet.h>
// #include "dsglobal.h"

#include <QGuiApplication>

class QDBusMessage;
namespace dock
{

class SearchItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool grandSearchVisible READ grandSearchVisible NOTIFY grandSearchVisibleChanged)
public:
    explicit SearchItem(QObject *parent = nullptr);

    Q_INVOKABLE void toggleGrandSearch();
    Q_INVOKABLE void toggleGrandSearchConfig();

    inline bool visible() const
    {
        return m_visible;
    }
    Q_INVOKABLE void setVisible(bool visible);

    Q_INVOKABLE bool grandSearchVisible() const
    {
        return m_grandSearchVisible;
    }

    bool load() override
    {
        return QGuiApplication::platformName() != QStringLiteral("wayland");
    }

Q_SIGNALS:
    void visibleChanged(bool);
    void grandSearchVisibleChanged(bool);

private slots:
    void onGrandSearchVisibleChanged(bool);

private:
    bool m_visible;
    bool m_grandSearchVisible;
};

}
