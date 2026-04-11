// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

// 缩略图模块统一头文件
// 包含所有缩略图相关的类

#include "thumbnailgenerator.h"
#include "thumbnailcache.h"
#include "thumbnailtaskmanager.h"
#include "thumbnailprovider.h"

// 内置生成器
#include "generators/imagethumbnailgenerator.h"
#include "generators/textthumbnailgenerator.h"

namespace GrandSearch {

/**
 * @brief 初始化缩略图模块
 *
 * 注册所有内置的缩略图生成器。
 * 应在应用启动时调用。
 */
inline void initThumbnailModule()
{
    // 注册内置生成器
    static ImageThumbnailGenerator imageGenerator;
    static TextThumbnailGenerator textGenerator;

    ThumbnailProvider::instance()->registerGenerator(&imageGenerator);
    ThumbnailProvider::instance()->registerGenerator(&textGenerator);
}

}   // namespace GrandSearch

#endif   // THUMBNAIL_H
