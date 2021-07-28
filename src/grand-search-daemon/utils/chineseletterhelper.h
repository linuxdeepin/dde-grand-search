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
#ifndef CHINESELETTERHELPER_H
#define CHINESELETTERHELPER_H

#include <QObject>

namespace GrandSearch {

class ChineseLetterHelper
{
public:
    static int convertChineseLetter2Pinyin(const QString &inStr, QSet<QString> &outFirstPys, QSet<QString> &outFullPys);
    static QString convertChineseName2Pinyin(const QString &inStr, bool isLastName);
    static void chineseNameSplit(const QString &inFullName, QString &outLastName, QString &outFirstName);
private:
    static int getPinyinByWord(const QString &inWord, QStringList &outFirstPy, QStringList &outFullPy);
    static QString getNoRepeatPinyinByWord(const QString &inWord);
};

}

#endif // CHINESELETTERHELPER_H
