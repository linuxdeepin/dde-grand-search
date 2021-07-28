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
#ifndef CHINESELETTERDICT_H
#define CHINESELETTERDICT_H

#include <QObject>

#define LASTNAME_TABLE_LENGTH           554
#define MULTITONEWORD_TABLE_LENGTH      708
#define CHINESELETTER_TABLE_LENGTH      411
#define COMPOUNDSURNAME_TABLE_LENGTH    90

namespace GrandSearch {
// 汉字-拼音
struct ChineseLetterTable {
    QString firstPinyin;        // 首字母
    QString fullPinyin;         // 全拼
    QString chinese;            // 汉字
};

// 特殊姓氏发音
struct LastNameTable {
    QString chinese;
    QString pinyi;
};

// 常用多音字
struct MultiToneWordTable {
    QString chinese;
    QString pinyin;
};

class ChineseLetterDict
{
public:
    // 获取汉字表
    static const ChineseLetterTable *getChineseLetterTable();

    // 获取姓氏表
    static const LastNameTable *getLastNameTable();

    // 获取多音字表
    static const MultiToneWordTable *getMultiToneWordTable();

    // 获取复姓表
    static const QString *getCompoundSurnameTable();
};

}

#endif // CHINESELETTERDICT_H
