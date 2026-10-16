// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHPLUGINADAPTOR_H
#define SEARCHPLUGINADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QObject>

// DBus 适配层：把 ConvertSearch 的方法通过会话总线暴露。
// 接口名与 convert-search.conf 中的 DBusInterface 保持一致。
class SearchPluginAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde-grand-search.convert.SearchPlugin")
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
