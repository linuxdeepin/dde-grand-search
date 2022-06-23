/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef AUDIOVIEW_H
#define AUDIOVIEW_H

#include "previewplugin.h"

#include <QLabel>

namespace GrandSearch {
namespace audio_preview {

class NameLabel: public QLabel
{
    Q_OBJECT
public:
    explicit NameLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SizeLabel: public QLabel
{
    Q_OBJECT
public:
    explicit SizeLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class AudioView : public QWidget
{
    Q_OBJECT
public:
    explicit AudioView(QWidget *parent = nullptr);
    void setItemInfo(const ItemInfo &item);
private:
    void initUI();
    QIcon defaultIcon(const QString &fileName);
private:
    QLabel *m_iconLabel = nullptr;
    NameLabel *m_nameLabel = nullptr;
    SizeLabel *m_sizeLabel = nullptr;
};

}}

#endif // AUDIOVIEW_H
