// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "global/commontools.h"
#include "configuration/configer.h"
#include "configuration/configer_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QSettings>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>

GRANDSEARCH_USE_NAMESPACE

// ---------------------------------------------------------------------------
// Configer tests
// ---------------------------------------------------------------------------

TEST(Configer, ut_constructor)
{
    Configer conf;
    ASSERT_NE(conf.d, nullptr);
    EXPECT_TRUE(conf.d->m_delayLoad.isSingleShot());
    EXPECT_EQ(conf.d->m_delayLoad.interval(), 50);
    // q should point back to the Configer that owns this private
    EXPECT_EQ(conf.d->q, &conf);
}

TEST(Configer, ut_destructor)
{
    Configer *conf = new Configer();
    ASSERT_NE(conf->d, nullptr);
    delete conf;
    // Reaching here without crash means the destructor worked.
    SUCCEED();
}

TEST(Configer, ut_instance)
{
    Configer *inst = Configer::instance();
    EXPECT_NE(inst, nullptr);
    // instance() always returns the same global singleton
    EXPECT_EQ(Configer::instance(), inst);
}

TEST(Configer, ut_initDefault)
{
    Configer conf;
    ASSERT_EQ(conf.d->m_root.get(), nullptr);
    conf.initDefault();

    ASSERT_NE(conf.d->m_root.get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_PREF_SEARCHERENABLED).get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_CLASS_FILE_DEEPIN).get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_TAILER_GROUP).get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_BLACKLIST_GROUP).get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_WEB_GROUP).get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_SEMANTIC_GROUP).get(), nullptr);
}

TEST(Configer, ut_init)
{
    Configer conf;
    stub_ext::StubExt st;
    bool init = false;
    st.set_lamda(&Configer::initDefault, [&init]() {
        init = true;
    });

    bool load = false;
    st.set_lamda(&Configer::onLoadConfig, [&load]() {
        load = true;
    });

    auto orgFunc = (bool (QFileInfo::*)() const) & QFileInfo::exists;
    st.set_lamda(orgFunc, []() {
        return true;
    });

    ASSERT_EQ(conf.d->m_watcher, nullptr);
    ASSERT_TRUE(conf.d->m_configPath.isEmpty());

    //测试更换&释放 watcher
    auto watcher = new QFileSystemWatcher;
    conf.d->m_watcher = watcher;

    EXPECT_TRUE(conf.init());
    EXPECT_NE(conf.d->m_watcher, nullptr);
    EXPECT_FALSE(conf.d->m_configPath.isEmpty());
    EXPECT_TRUE(init);
    EXPECT_TRUE(load);
}

TEST(Configer, ut_group)
{
    Configer conf;
    ASSERT_EQ(conf.d->m_root.get(), nullptr);
    EXPECT_EQ(conf.group("test"), nullptr);

    conf.d->m_root.reset(new UserPreference({}));
    UserPreferencePointer va(new UserPreference({}));
    conf.d->m_root->setValue("test", QVariant::fromValue(va));

    EXPECT_EQ(conf.group("test"), va.get());
}

TEST(Configer, ut_onFileChanged)
{
    Configer conf;
    ASSERT_FALSE(conf.d->m_delayLoad.isActive());
    conf.onFileChanged("/tmp/test.conf");
    EXPECT_TRUE(conf.d->m_delayLoad.isActive());
}

TEST(Configer, ut_onLoadConfig_emptyPath)
{
    Configer conf;
    ASSERT_TRUE(conf.d->m_configPath.isEmpty());
    conf.onLoadConfig();
    // Should return early without crash; m_root remains unset
    EXPECT_EQ(conf.d->m_root.get(), nullptr);
}

TEST(Configer, ut_onLoadConfig_fileNotFound)
{
    Configer conf;
    conf.d->m_configPath = "/nonexistent/path/to/config.conf";
    conf.onLoadConfig();
    // Should return early; m_root remains unset
    EXPECT_EQ(conf.d->m_root.get(), nullptr);
}

TEST(Configer, ut_onLoadConfig_validFile)
{
    Configer conf;
    conf.initDefault();
    ASSERT_NE(conf.d->m_root.get(), nullptr);

    // Create a temporary config file with version info
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString configPath = tmpDir.path() + "/test.conf";
    {
        QSettings set(configPath, QSettings::IniFormat);
        set.beginGroup("Version_Group");
        set.setValue("version.config", "1.0");
        set.endGroup();
        set.sync();
    }
    ASSERT_TRUE(QFileInfo::exists(configPath));

    conf.d->m_configPath = configPath;
    conf.onLoadConfig();
    // After successful load, m_root should still be valid (updateConfig1 called)
    EXPECT_NE(conf.d->m_root.get(), nullptr);
}

TEST(Configer, ut_onLoadConfig_noVersion)
{
    Configer conf;
    conf.initDefault();

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString configPath = tmpDir.path() + "/noversion.conf";
    {
        QSettings set(configPath, QSettings::IniFormat);
        set.setValue("some/key", "value");
        set.sync();
    }
    ASSERT_TRUE(QFileInfo::exists(configPath));

    conf.d->m_configPath = configPath;
    conf.onLoadConfig();
    // Should skip loading because version info is missing
    EXPECT_NE(conf.d->m_root.get(), nullptr);
}

// ---------------------------------------------------------------------------
// ConfigerPrivate tests
// ---------------------------------------------------------------------------

TEST(ConfigerPrivate, ut_constructor)
{
    Configer conf;
    ASSERT_NE(conf.d, nullptr);
    EXPECT_EQ(conf.d->q, &conf);
}

TEST(ConfigerPrivate, ut_blacklist)
{
    auto up = ConfigerPrivate::blacklist();
    ASSERT_NE(up.get(), nullptr);
    // blacklist should contain the GRANDSEARCH_BLACKLIST_PATH key
    QVariant var;
    EXPECT_TRUE(up->innerValue(GRANDSEARCH_BLACKLIST_PATH, var));
    EXPECT_EQ(var.toStringList(), QStringList(""));
}

TEST(ConfigerPrivate, ut_defaultSearcher)
{
    auto up = ConfigerPrivate::defaultSearcher();
    ASSERT_NE(up.get(), nullptr);
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_FILE_DEEPIN, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_FILE_FULLTEXT, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_OCR_TEXT, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_APP_DESKTOP, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_WEB_STATICTEXT, false));
}

TEST(ConfigerPrivate, ut_fileSearcher)
{
    auto up = ConfigerPrivate::fileSearcher();
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FOLDER, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_VIDEO, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_AUDIO, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_PICTURE, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false));
}

TEST(ConfigerPrivate, ut_semanticEngine)
{
    auto up = ConfigerPrivate::semanticEngine();
    ASSERT_NE(up.get(), nullptr);
    EXPECT_TRUE(up->value(GRANDSEARCH_SEMANTIC_ENABLED, false));
}

TEST(ConfigerPrivate, ut_tailerData)
{
    auto up = ConfigerPrivate::tailerData();
    ASSERT_NE(up.get(), nullptr);
    EXPECT_FALSE(up->value(GRANDSEARCH_TAILER_PARENTDIR, true));
}

TEST(ConfigerPrivate, ut_webSearchEngine)
{
    auto up = ConfigerPrivate::webSearchEngine();
    ASSERT_NE(up.get(), nullptr);
    EXPECT_EQ(up->value(GRANDSEARCH_WEB_SEARCHENGINE, QString("nonempty")), QString(""));
}

TEST(ConfigerPrivate, ut_updateConfig1)
{
    Configer conf;
    EXPECT_FALSE(conf.d->updateConfig1(nullptr));
    QSettings set;
    EXPECT_FALSE(conf.d->updateConfig1(&set));

    conf.initDefault();
    EXPECT_TRUE(conf.d->updateConfig1(&set));
}

TEST(ConfigerPrivate, ut_updateConfig1_withValues)
{
    Configer conf;
    conf.initDefault();

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString configPath = tmpDir.path() + "/test.conf";
    {
        QSettings set(configPath, QSettings::IniFormat);
        set.beginGroup(GRANDSEARCH_SEARCH_GROUP);
        set.setValue(GRANDSEARCH_GROUP_FOLDER, false);
        set.setValue(GRANDSEARCH_GROUP_FILE, true);
        set.setValue(GRANDSEARCH_GROUP_FILE_VIDEO, false);
        set.setValue(GRANDSEARCH_GROUP_FILE_AUDIO, true);
        set.setValue(GRANDSEARCH_GROUP_FILE_PICTURE, false);
        set.setValue(GRANDSEARCH_GROUP_FILE_DOCUMNET, true);
        set.setValue(GRANDSEARCH_GROUP_SETTING, false);
        set.setValue(GRANDSEARCH_GROUP_APP, false);
        set.setValue(GRANDSEARCH_GROUP_WEB, false);
        set.endGroup();
        set.sync();
    }

    QSettings set(configPath, QSettings::IniFormat);
    EXPECT_TRUE(conf.d->updateConfig1(&set));

    // Verify the file search sub-config was updated
    auto fileConf = conf.d->m_root->group(GRANDSEARCH_CLASS_FILE_DEEPIN);
    ASSERT_NE(fileConf.get(), nullptr);
    EXPECT_FALSE(fileConf->value(GRANDSEARCH_GROUP_FOLDER, true));
    EXPECT_TRUE(fileConf->value(GRANDSEARCH_GROUP_FILE, false));
    EXPECT_FALSE(fileConf->value(GRANDSEARCH_GROUP_FILE_VIDEO, true));
    EXPECT_TRUE(fileConf->value(GRANDSEARCH_GROUP_FILE_AUDIO, false));

    // Verify searcher toggles
    auto searcherConf = conf.d->m_root->group(GRANDSEARCH_PREF_SEARCHERENABLED);
    ASSERT_NE(searcherConf.get(), nullptr);
    // Setting and app search should be off
    EXPECT_FALSE(searcherConf->value(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, true));
    EXPECT_FALSE(searcherConf->value(GRANDSEARCH_CLASS_APP_DESKTOP, true));
    EXPECT_FALSE(searcherConf->value(GRANDSEARCH_CLASS_WEB_STATICTEXT, true));
}

TEST(ConfigerPrivate, ut_resetPath)
{
    stub_ext::StubExt st;
    st.set_lamda(CommonTools::bindPathTransform, []() { return "/data/home"; });

    Configer conf;
    QString path("/");
    conf.d->resetPath(path);
    EXPECT_EQ(QString("/"), path);
    path = QString("/data/home");
    conf.d->resetPath(path);
    EXPECT_EQ(QString("/home/"), path);
    path = QString("/data/home/aaa");
    conf.d->resetPath(path);
    EXPECT_EQ(QString("/home/aaa/"), path);
}
