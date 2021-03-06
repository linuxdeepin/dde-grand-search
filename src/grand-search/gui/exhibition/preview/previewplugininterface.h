/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
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

#ifndef PREVIEWPLUGINIFACE_H
#define PREVIEWPLUGINIFACE_H

#include <QObject>

namespace GrandSearch {
#define FilePreviewInterface_iid "com.deepin.grandsearch.FilePreviewInterface.iid"

class PreviewPlugin;
class PreviewPluginInterface
{
public:
    virtual PreviewPlugin *create(const QString &mimetype) = 0;
};

}

Q_DECLARE_INTERFACE(GrandSearch::PreviewPluginInterface, FilePreviewInterface_iid)
#endif // PREVIEWPLUGINIFACE_H
