/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "fsearch_utils.h"
#include "utils/chineseletterhelper.h"

#include <QRegExp>

#include <stdio.h>

bool convert_chinese_2_pinyin(const char *in_str, char *out_first_py, char *out_full_py)
{
    QRegExp reg("[\u4e00-\u9fa5]");
    QString str(in_str);
    if (!str.contains(reg))
        return false;

    QString first_py;
    QString full_py;
    bool ret = GrandSearch::ChineseLetterHelper::instance()->convertChinese2Pinyin(str, first_py, full_py);
    if (ret) {
        int full_py_len = full_py.toLocal8Bit().size();
        int first_py_len = first_py.toLocal8Bit().size();

        memcpy(out_full_py, full_py.toLocal8Bit().data(), full_py_len > FILENAME_MAX ? FILENAME_MAX : static_cast<size_t>(full_py_len));
        memcpy(out_first_py, first_py.toLocal8Bit().data(), first_py_len > FILENAME_MAX ? FILENAME_MAX : static_cast<size_t>(first_py_len));
    }

    return ret;
}
