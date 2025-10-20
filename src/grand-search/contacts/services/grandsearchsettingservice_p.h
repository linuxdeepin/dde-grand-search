// SPDX-FileCopyrightText: 2021 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHSETTINGSERVICE_P_H
#define GRANDSEARCHSETTINGSERVICE_P_H

#include "grandsearchsettingservice.h"

#include <QTimer>

namespace GrandSearchSetting {

class GrandSearchSettingServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit GrandSearchSettingServicePrivate(GrandSearchSettingService *parent = nullptr);

    GrandSearchSettingService *q_p = nullptr;
    QTimer *timer = nullptr;
};

}

#endif // GRANDSEARCHSETTINGSERVICE_P_H
