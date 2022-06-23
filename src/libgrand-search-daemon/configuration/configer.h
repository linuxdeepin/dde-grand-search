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
#ifndef CONFIGER_H
#define CONFIGER_H

#include "userpreference.h"

#include <QString>

#define ConfigerIns  GrandSearch::Configer::instance()

namespace GrandSearch {

class ConfigerPrivate;
class Configer : public QObject
{
    Q_OBJECT
    friend class ConfigerPrivate;
public:
    static Configer *instance();
    bool init();
    UserPreferencePointer group(const QString &name) const;
protected:
    void initDefault();
protected slots:
    void onFileChanged(const QString &path);
    void onLoadConfig();
protected:
    explicit Configer(QObject *parent = nullptr);
    ~Configer();
private:
    ConfigerPrivate *d;
};

}

#endif // CONFIGER_H
