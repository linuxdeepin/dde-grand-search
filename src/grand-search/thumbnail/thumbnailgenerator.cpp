// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailgenerator.h"

#include <QFileInfo>

namespace GrandSearch {

bool ThumbnailGenerator::isFileSizeValid(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }

    return fileInfo.size() <= maxFileSize();
}

} // namespace GrandSearch
