// SPDX-FileCopyrightText: 2021 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHSETTINGSERVICE_H
#define GRANDSEARCHSETTINGSERVICE_H

#include <QObject>
#include <QScopedPointer>

#define GrandSearchSettingServiceName          "com.deepin.dde.GrandSearchSetting"
#define GrandSearchSettingServicePath          "/com/deepin/dde/GrandSearchSetting"
#define GrandSearchSettingServiceInterface     "com.deepin.dde.GrandSearchSetting"

namespace GrandSearchSetting {

class GrandSearchSettingServicePrivate;
class GrandSearchSettingService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", GrandSearchSettingServiceInterface)
public:
    explicit GrandSearchSettingService(QObject *parent = nullptr);
    ~GrandSearchSettingService();

    Q_SCRIPTABLE void Show();

private:
    QScopedPointer<GrandSearchSettingServicePrivate> d_p;
};

}

#endif // GRANDSEARCHSERVICE_H
