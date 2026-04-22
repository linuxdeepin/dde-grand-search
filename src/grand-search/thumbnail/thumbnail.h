// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

// 缩略图模块统一头文件
// 包含所有缩略图相关的类

namespace GrandSearch {

/**
 * @brief 缩略图尺寸枚举（与文管保持一致）
 *
 * 遵循 XDG 缩略图标准，分为四个尺寸等级：
 * - Small: 小尺寸 (<= 128x128)
 * - Normal: 普通尺寸 (<= 256x256)
 * - Large: 大尺寸 (<= 512x512)
 * - XLarge: 超大尺寸 (<= 1024x1024)
 */
enum class ThumbnailSize {
    Small,    // 小尺寸 (<= 128x128)
    Normal,   // 普通尺寸 (<= 256x256)
    Large,    // 大尺寸 (<= 512x512)
    XLarge    // 超大尺寸 (<= 1024x1024)
};

}

// 包含其他头文件（放在枚举定义之后，避免循环依赖）
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
