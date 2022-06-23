/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef SEARCHCONFIG_H
#define SEARCHCONFIG_H

#include <QObject>
#include <QSettings>
#include <QMutex>

namespace GrandSearch {

class SearchConfig : public QObject
{
    Q_OBJECT
public:
    static SearchConfig *instance();

    QVariant getConfig(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());
    void setConfig(const QString &group, const QString &key, const QVariant &value);
    void removeConfig(const QString &group, const QString &key);
    void setConfigList(const QString &group, const QStringList &keys, const QVariantList &values);
    void removeConfigList(const QString &group, const QStringList &keys);

protected:
    explicit SearchConfig();
    ~SearchConfig();

private:
    void convertConfigFile();

private:
    QMutex  m_mutex;
    QSettings *m_settings = nullptr;
};

}

#endif // SEARCHCONFIG_H
