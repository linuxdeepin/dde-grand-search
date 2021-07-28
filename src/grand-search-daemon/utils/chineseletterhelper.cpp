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
#include "chineseletterhelper.h"
#include "chineseletterdict.h"

#include <QStringList>
#include <QVector>
#include <QDebug>

namespace GrandSearch {

int ChineseLetterHelper::convertChineseLetter2Pinyin(const QString &inStr, QSet<QString> &outFirstPys, QSet<QString> &outFullPys)
{
    outFirstPys.clear();
    outFullPys.clear();
    if (inStr.isEmpty())
        return -1;

    // 获取每个字的所有读音
    int wordNum = inStr.size();
    QVector<QStringList> firstPys;
    QVector<QStringList> fullPys;
    for (int i = 0; i < wordNum; ++i) {
        QStringList firstPy;
        QStringList fullPy;
        getPinyinByWord(inStr.at(i), firstPy, fullPy);

        firstPys.push_back(firstPy);
        fullPys.push_back(fullPy);
    }

    // 开始排列组合
    QVector<int> nowIndexList;
    QVector<int> maxIndexList;
    int sumMaxIndex = 0;
    for (int i = 0; i < wordNum; ++i) {
        nowIndexList.push_back(0);
        maxIndexList.push_back(fullPys[i].size() - 1);
        sumMaxIndex += maxIndexList[i];
    }

    QString firstPy;
    QString fullPy;
    // 第一次组合（所有采用第一个）
    bool isNeedCombination = sumMaxIndex > 0 ? true : false;
    for (int i = 0; i < wordNum; ++i) {
        firstPy += firstPys[i][0];
        fullPy += fullPys[i][0];
    }
    outFirstPys << firstPy;
    outFullPys << fullPy;

    // 循环遍历
    while (isNeedCombination) {
        // 组合排列
        bool isAlreadyRunOnce = false;
        for (int i = 0; i < wordNum; ++i) {
            if (isAlreadyRunOnce)
                break;

            if (maxIndexList[i] != 0) {
                if (nowIndexList[i] < maxIndexList[i]) {
                    isAlreadyRunOnce = true;
                    nowIndexList[i]++;
                } else if (nowIndexList[i] == maxIndexList[i]) {
                    nowIndexList[i] = 0;
                }
            }
        }

        // 组合输出字符
        firstPy.clear();
        fullPy.clear();
        for (int i = 0; i < wordNum; ++i) {
            firstPy += firstPys[i][nowIndexList[i]];
            fullPy += fullPys[i][nowIndexList[i]];
        }
        outFirstPys << firstPy;
        outFullPys << fullPy;

        bool canOut = true;
        for (int i = 0; i < wordNum; ++i) {
            if (nowIndexList[i] != maxIndexList[i]) {
                canOut = false;
                break;
            }
        }

        if (canOut)
            break;
    }

    return 0;
}

QString ChineseLetterHelper::convertChineseName2Pinyin(const QString &inStr, bool isLastName)
{
    QString outStr;

    //姓氏
    if (isLastName) {
        //在百家姓中查找
        auto *plnTable = ChineseLetterDict::getLastNameTable();
        for (int i = 0; i < LASTNAME_TABLE_LENGTH; i++) {
            if (inStr == plnTable[i].chinese) {
                outStr = plnTable[i].pinyi;
                break;
            }
        }
    }

    //直接汉字库查找
    if (outStr.isEmpty()) {
        for (int i = 0; i < inStr.size(); i++) {
            outStr += getNoRepeatPinyinByWord(inStr[i]);
        }
    }

    return outStr;
}

void ChineseLetterHelper::chineseNameSplit(const QString &inFullName, QString &outLastName, QString &outFirstName)
{
    //如果有非汉字字符，直接返回
    for (int i = 0; i < inFullName.length(); i++) {
        ushort uni = inFullName.at(i).unicode();
        if (uni < 0x4E00 || uni > 0x9FA5) {
            outLastName = "";
            outFirstName = inFullName;
            return;
        }
    }

    //
    outLastName = "";
    outFirstName = "";

    if (inFullName.isEmpty()) {
        return;
    } else if (inFullName.length() == 1) {
        outFirstName = inFullName;
    } else if (inFullName.length() == 2) {
        outLastName = inFullName.at(0);
        outFirstName = inFullName.at(1);
    } else {
        QString guessLastName = inFullName.mid(0, 2);
        auto *pcsTable = ChineseLetterDict::getCompoundSurnameTable();
        for (int i = 0; i < COMPOUNDSURNAME_TABLE_LENGTH; ++i) {
            if (guessLastName == pcsTable[i]) {
                outLastName = pcsTable[i];
                outFirstName = inFullName.mid(2);
                break;
            }
        }

        if (outLastName.isEmpty()) {
            if (inFullName.length() == 4) {
                outLastName = inFullName.mid(0, 2);
                outFirstName = inFullName.mid(2);
            } else {
                outLastName = inFullName.at(0);
                outFirstName = inFullName.mid(1);
            }
        }
    }
}

int ChineseLetterHelper::getPinyinByWord(const QString &inWord, QStringList &outFirstPy, QStringList &outFullPy)
{
    if (inWord.size() > 1)
        return -1;

    outFullPy.clear();
    outFirstPy.clear();

    ushort uni = inWord[0].unicode();
    if (uni >= 0x4E00 && uni <= 0x9FA5) {
        auto *pclTable = ChineseLetterDict::getChineseLetterTable();
        for (int index = 0; index < CHINESELETTER_TABLE_LENGTH; ++index) {
            if (pclTable[index].chinese.indexOf(uni, 0, Qt::CaseSensitive) != -1) {
                outFirstPy.append(pclTable[index].firstPinyin);
                outFullPy.append(pclTable[index].fullPinyin);
            }
        }
    }

    if (outFirstPy.isEmpty())
        outFirstPy.append(inWord);

    if (outFullPy.isEmpty())
        outFullPy.append(inWord);

    return 0;
}

QString ChineseLetterHelper::getNoRepeatPinyinByWord(const QString &inWord)
{
    QString outPingyin;

    if (inWord.size() > 1) {
        return inWord;
    }

    //多音字里面查询
    auto *pmtwTable = ChineseLetterDict::getMultiToneWordTable();
    for (int i = 0; i < MULTITONEWORD_TABLE_LENGTH; ++i) {
        if (inWord == pmtwTable[i].chinese) {
            outPingyin = pmtwTable[i].pinyin;
            break;
        }
    }

    if (!outPingyin.isEmpty())
        return outPingyin;

    //全字库查找
    ushort uni = inWord[0].unicode();
    if (uni >= 0x4E00 && uni <= 0x9FA5) {
        auto *pclTable = ChineseLetterDict::getChineseLetterTable();
        for (int index = 0; index < CHINESELETTER_TABLE_LENGTH; ++index) {
            if (pclTable[index].chinese.indexOf(uni, 0, Qt::CaseSensitive) != -1) {
                outPingyin = pclTable[index].fullPinyin;
                break;
            }
        }
    }

    if (outPingyin.isEmpty())
        outPingyin = inWord;

    return outPingyin;
}

}
