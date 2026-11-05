// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORPROVIDER_H
#define COLORPROVIDER_H

#include <QString>
#include <QList>

#include "resultbuilder.h"

// 颜色转换：HEX / RGB / HSL 三向互转（设计、办公友好）。
class ColorProvider {
public:
    static QList<ResultBuilder::Item> convert(const QString &color);
};

#endif // COLORPROVIDER_H
