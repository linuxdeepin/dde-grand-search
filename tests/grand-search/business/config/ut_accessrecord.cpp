/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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

#include "business/config/accessrecord/accessrecord.h"
#include "global/matcheditem.h"
#include "global/searchconfigdefine.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QJsonObject>
#include <QIODevice>
#include <QFile>
#include <QtCore/qfiledevice.h>
#include <QtCore/qstring.h>
#include <stdio.h>

#include <QTest>

TEST(AccessRecordTest, init)
{
    AccessRecord *accessRecord = new AccessRecord;

    EXPECT_TRUE(accessRecord);

    delete accessRecord;
}

TEST(AccessRecordTest, startParseRecord)
{
    auto recordPath = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first();
    AccessRecord::instance()->m_recordPath = recordPath
            + "/" + QApplication::organizationName()
            + "/" + GRANDSEARCH_NAME
            +"/" + "accessrecord.json";
    QFileInfo file(recordPath);
    QFile file1(recordPath);
    file1.remove();
    AccessRecord::instance()->startParseRecord();
    bool result = file.exists();
    EXPECT_TRUE(result);
}

TEST(AccessRecordTest, updateRecord)
{
    GrandSearch::MatchedItem item;
    item.searcher = "test";
    item.item = "test1";
    qint64 time = 1;

    AccessRecord::instance()->m_recordObj.insert("version", "1.0");

    // ????????????????????????searcher
    QJsonObject recordObjNew;
    QJsonObject itemObj;
    QJsonArray array = {1};
    itemObj.insert("test1", array);
    recordObjNew.insert("test", itemObj);
    recordObjNew.insert("version", "1.0");

    AccessRecord::instance()->updateRecord(item, time);
    EXPECT_EQ(AccessRecord::instance()->m_recordObj, recordObjNew);

    // ?????????????????????searcher???????????????item
    item.item = "test2";
    itemObj.insert("test2", array);
    recordObjNew.insert("test", itemObj);
    AccessRecord::instance()->updateRecord(item, time);
    EXPECT_EQ(AccessRecord::instance()->m_recordObj, recordObjNew);

    // ?????????????????????searcher????????????item
    time = 2;
    array = {1, 2};
    itemObj.insert("test2", array);
    recordObjNew.insert("test", itemObj);
    AccessRecord::instance()->updateRecord(item, time);
    EXPECT_EQ(AccessRecord::instance()->m_recordObj, recordObjNew);
}

TEST(AccessRecordTest, sync)
{
    AccessRecord accessRecord;
    accessRecord.m_finished = true;
    accessRecord.m_recordPath = "./test";
    QFile file(accessRecord.m_recordPath);
    file.open(QFile::Truncate);
    file.close();

    accessRecord.m_recordObj.insert("aaa", "1");
    accessRecord.sync();
    file.open(QFile::ReadOnly);
    QByteArray array = file.readAll();
    file.close();
    QJsonDocument doc(QJsonDocument::fromJson(array));
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj, accessRecord.m_recordObj);

}

TEST(AccessRecordTest, parseRecord)
{
    // ???parseJson?????????????????????????????????
    bool parse = false;
    stub_ext::StubExt stu;
    stu.set_lamda(&AccessRecord::parseJson, [&]() {
        parse = true;
    });

    // ???????????????
    QFile tempFile(AccessRecord::instance()->m_recordPath);
    tempFile.remove();

    AccessRecord::parseRecord(AccessRecord::instance()->m_recordPath);

    auto recordPath = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first();
    AccessRecord::instance()->m_recordPath = recordPath
            + "/" + QApplication::organizationName()
            + "/" + GRANDSEARCH_NAME
            +"/" + "accessrecord.json";
    QFileInfo file(recordPath);
    bool result = file.exists();
    EXPECT_TRUE(result);
    EXPECT_TRUE(parse);
}

TEST(AccessRecordTest, parseJson)
{
    // ????????????????????????json??????
    QJsonObject searcher;
    QJsonObject item;
    QJsonArray time;
    time.append(1647743325);
    const qint64 currentTimeT = QDateTime::currentDateTime().toSecsSinceEpoch();
    time.append(currentTimeT);
    item.insert("test1", time);
    searcher.insert("a", item);
    time.removeLast();
    item.insert("test1", time);
    item.insert("test2", time);
    searcher.insert("b", item);
    time.removeLast();
    for (int i = 0; i < 21; ++i) {
        time.append(currentTimeT);
    }
    time.append(1647743200);
    item.insert("test3", time);
    searcher.insert("c", item);
    searcher.insert("version", "1.0");
    QJsonDocument doc(searcher);
    QByteArray data = doc.toJson();

    // ???????????????????????????
    stub_ext::StubExt stu;
    stu.set_lamda(&QFile::readAll, [&]() {
        return data;
    });

    AccessRecord::parseJson(AccessRecord::instance()->m_recordPath);

    QJsonObject searcherNew;
    QJsonObject itemNew;
    QJsonArray timeNew;
    timeNew.append(currentTimeT);
    itemNew.insert("test1", timeNew);
    searcherNew.insert("a", itemNew);
    timeNew.removeLast();
    for (int i = 0; i < 21; ++i) {
        timeNew.append(currentTimeT);
    }
    QJsonObject itemAnotherNew;
    itemAnotherNew.insert("test3", timeNew);
    searcherNew.insert("c", itemAnotherNew);
    searcherNew.insert("version", "1.0");

    QHash<QString, QHash<QString, int>> searcherHash;
    QHash<QString, int> itemHash;
    itemHash.insert("test1", 1);
    searcherHash.insert("a", itemHash);
    itemHash.clear();
    itemHash.insert("test3", 20);
    searcherHash.insert("c", itemHash);

    AccessRecord::parseJson(AccessRecord::instance()->m_recordPath);
    EXPECT_EQ(searcherNew, AccessRecord::instance()->m_recordObj);
    EXPECT_EQ(searcherHash, AccessRecord::instance()->m_recordHash);
}
