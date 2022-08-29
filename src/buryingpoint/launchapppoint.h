// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LAUNCHAPPPOINT_H
#define LAUNCHAPPPOINT_H

#include "basicpoint.h"

#include <QCoreApplication>

namespace GrandSearch {

namespace burying_point {

class LaunchAppPoint : public BasicPoint
{
public:
    explicit LaunchAppPoint();
    ~LaunchAppPoint() override;

    QVariantMap assemblingData() const override;

private:
    QPair<QString, QString> m_version;
    QPair<QString, int> m_mode;
};

}

}
#endif // LAUNCHAPPPOINT_H
