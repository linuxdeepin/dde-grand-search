// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHPLUGINADAPTOR_H
#define SEARCHPLUGINADAPTOR_H

#include <QtDBus/QDBusAbstractAdaptor>

class SearchPluginAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.example.CalculatorSearch.SearchPlugin")
    Q_CLASSINFO("D-Bus Introspection", ""
        "  <interface name=\"com.example.CalculatorSearch.SearchPlugin\">\n"
        "    <method name=\"Search\">\n"
        "      <arg type=\"s\" direction=\"out\"/>\n"
        "      <arg name=\"json\" type=\"s\" direction=\"in\"/>\n"
        "    </method>\n"
        "    <method name=\"Action\">\n"
        "      <arg type=\"b\" direction=\"out\"/>\n"
        "      <arg name=\"json\" type=\"s\" direction=\"in\"/>\n"
        "    </method>\n"
        "    <method name=\"Stop\">\n"
        "      <arg type=\"b\" direction=\"out\"/>\n"
        "      <arg name=\"json\" type=\"s\" direction=\"in\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "")

public:
    explicit SearchPluginAdaptor(QObject *parent = nullptr);
    ~SearchPluginAdaptor() override;

public slots:
    QString Search(const QString &json);
    bool Stop(const QString &json);
    bool Action(const QString &json);

private:
    QObject *m_parent;
};

#endif // SEARCHPLUGINADAPTOR_H
