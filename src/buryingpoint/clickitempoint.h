// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLICKITEMPOINT_H
#define CLICKITEMPOINT_H

#include "basicpoint.h"

namespace GrandSearch {

namespace burying_point {

class ClickItemPoint : public BasicPoint
{
public:
    explicit ClickItemPoint();
    ~ClickItemPoint() override;

    QVariantMap assemblingData() const override;

private:
    QPair<QString, QString> m_version;
};

}

}
#endif // CLICKITEMPOINT_H
