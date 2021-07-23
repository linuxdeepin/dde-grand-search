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
#ifndef CONVERTORV1_0_H
#define CONVERTORV1_0_H

#include "abstractconvertor.h"

class ConvertorV1_0 : public AbstractConvertor
{
public:
    explicit ConvertorV1_0();
    ~ConvertorV1_0();
    static AbstractConvertor *create();
    QString version() const Q_DECL_OVERRIDE;
    QHash<QString, ConvertInterface> interfaces() Q_DECL_OVERRIDE;
    static int search(QJsonObject *json, void *info);
    static int result(QJsonObject *json, void *info);
    static int stop(QJsonObject *json, void *info);
    static int action(QJsonObject *json, void *info);
};

#endif // CONVERTORV1_0_H
