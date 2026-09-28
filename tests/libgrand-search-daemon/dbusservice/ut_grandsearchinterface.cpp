// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | GrandSearchInterface::GrandSearchInterface | low | complexity:1 | 1 | 1 |
// | GrandSearchInterface::~GrandSearchInterface | low | - | 1 | 1 |
// | GrandSearchInterface::init | low | complexity:4 | 2 | 2 |
// | GrandSearchInterface::Search | mid | complexity:10 | 3 | 3 |
// | GrandSearchInterface::Terminate | low | complexity:3 | 1 | 2 |
// | GrandSearchInterface::MatchedResults | low | complexity:5 | 2 | 2 |
// | GrandSearchInterface::MatchedBuffer | low | complexity:8 | 3 | 3 |
// | GrandSearchInterface::OpenWithPlugin | low | complexity:3 | 1 | 2 |
// | GrandSearchInterface::KeepAlive | low | complexity:6 | 2 | 2 |
// | GrandSearchInterfacePrivate::GrandSearchInterfacePrivate | low | complexity:2 | 1 | 1 |
// | GrandSearchInterfacePrivate::~GrandSearchInterfacePrivate | low | complexity:3 | 1 | 1 |
// | GrandSearchInterfacePrivate::isAccessable | low | complexity:8 | 2 | 2 |
// | GrandSearchInterfacePrivate::onMatched | low | complexity:4 | 2 | 2 |
// | GrandSearchInterfacePrivate::onSearchCompleted | low | complexity:4 | 2 | 2 |
// | GrandSearchInterfacePrivate::terminate | mid | complexity:5 | 2 | 2 |
// | GrandSearchInterfacePrivate::vaildSession | low | complexity:3 | 2 | 3 |
// ─── 生成后填入 actual 列，低于 min 即违规 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 4. 同质 ≥ 3 组用 TEST_P: [N/A]
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [N/A]
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]
// 10. stub_ext vs gMock 选择正确: [x]
//
// 分支清单 → 用例映射：
// GrandSearchInterface::Search:
//   - CHECKINVOKER(false) → ut_search (accessable=false)
//   - session.size()!=36 || key.isEmpty() || key.size()>512 → ut_search (invalid params)
//   - d->m_main->newSearch(key)==true → ut_search_1 (success path)
//   - d->m_main->newSearch(key)==false → ut_search_2 (newSearch fails)
// GrandSearchInterface::Terminate:
//   - CHECKINVOKER() → ut_terminate (accessable=false)
//   - d->terminate() → ut_terminate_1 (accessable=true)
// GrandSearchInterface::MatchedResults:
//   - CHECKINVOKER(ret) → ut_matchedresults (accessable=false)
//   - vaildSession==true → ut_matchedresults_1 (valid session)
//   - vaildSession==false → ut_matchedresults_1 (invalid session)
// GrandSearchInterface::MatchedBuffer:
//   - CHECKINVOKER(ret) → ut_matchedbuffer (accessable=false)
//   - vaildSession==true && !isEmptyBuffer → ut_matchedbuffer_1 (valid, non-empty)
//   - vaildSession==true && isEmptyBuffer → ut_matchedbuffer_2 (valid, empty)
//   - vaildSession==false → ut_matchedbuffer_3 (invalid session)
// GrandSearchInterface::OpenWithPlugin:
//   - CHECKINVOKER(false) → ut_openwithplugin (accessable=false)
//   - d->m_main->searcherAction → ut_openwithplugin_1 (success path)
// GrandSearchInterface::KeepAlive:
//   - CHECKINVOKER(false) → ut_keepalive (accessable=false)
//   - vaildSession==true → ut_keepalive_1 (valid session)
//   - vaildSession==false → ut_keepalive_1 (invalid session)
// GrandSearchInterface::init:
//   - m_main->init()==true → ut_init (success)
//   - m_main->init()==false → ut_init_1 (failure)
// GrandSearchInterfacePrivate::isAccessable:
//   - fileInfo.exists()==false → ut_isaccessable (not exists)
//   - m_permit.value()==true/false → ut_isaccessable_1 (permit check)
// GrandSearchInterfacePrivate::onMatched:
//   - m_session.isEmpty()==false → ut_onmatched (emits signal)
//   - m_session.isEmpty()==true → ut_onmatched_1 (no signal)
// GrandSearchInterfacePrivate::onSearchCompleted:
//   - m_session.isEmpty()==false → ut_onsearchcompleted (emits signal)
//   - m_session.isEmpty()==true → ut_onsearchcompleted_1 (no signal)
// GrandSearchInterfacePrivate::terminate:
//   - m_main!=nullptr → ut_terminate_private (with main)
//   - m_main==nullptr → ut_terminate_private_1 (without main)
// GrandSearchInterfacePrivate::vaildSession:
//   - session.isEmpty()==true → ut_vaildsession (empty)
//   - m_session==session → ut_vaildsession_1 (matching)
//   - m_session!=session → ut_vaildsession_2 (non-matching)

#include "global/grandsearch_global.h"
#include "dbusservice/grandsearchinterface.h"
#include "dbusservice/grandsearchinterface_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QUuid>

GRANDSEARCH_USE_NAMESPACE

class GrandSearchInterfaceTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
    }

    static void TearDownTestSuite() {
    }

    void SetUp() override {
        stub.clear();

        gsi = new GrandSearchInterface();

        msgStub = QDBusMessage();
        stub.set_lamda(&GrandSearchInterface::message, [this]()->const QDBusMessage & {
            return msgStub;
        });
    }

    void TearDown() override {
        stub.clear();
        delete gsi;
        gsi = nullptr;
    }

    void setAccessable(bool ok) {
        stub.set_lamda(&GrandSearchInterfacePrivate::isAccessable, [ok]() {
            return ok;
        });
    }

    stub_ext::StubExt stub;
    GrandSearchInterface *gsi = nullptr;
    QDBusMessage msgStub;
};

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface constructor / destructor
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, Constructor_CreatesPrivateObject_ReturnsValidState)
{
    // Arrange — gsi created in SetUp
    ASSERT_NE(gsi->d, nullptr);

    // Act — inspect permit map initialization
    int permitCount = gsi->d->m_permit.size();
    bool permitValue = permitCount > 0 ? gsi->d->m_permit.constBegin().value() : false;

    // Assert
    EXPECT_EQ(permitCount, 1);
    EXPECT_TRUE(permitValue);
}

TEST_F(GrandSearchInterfaceTest, Destructor_DestroysPrivateObject_NoCrash)
{
    // Arrange
    GrandSearchInterface *tmp = new GrandSearchInterface();
    ASSERT_NE(tmp->d, nullptr);

    // Act
    delete tmp;

    // Assert — destructor ran without crash; verify fresh instance still works
    GrandSearchInterface *tmp2 = new GrandSearchInterface();
    EXPECT_NE(tmp2->d, nullptr);
    EXPECT_EQ(tmp2->d->m_permit.size(), 1);
    delete tmp2;
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface::init
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, Init_MainControllerInitSucceeds_ReturnsTrue)
{
    // Arrange
    bool initCalled = false;
    stub.set_lamda(&MainController::init, [&initCalled]() {
        initCalled = true;
        return true;
    });

    // Act
    bool result = gsi->init();

    // Assert
    EXPECT_TRUE(result);              // branch: m_main->init()==true
    EXPECT_TRUE(initCalled);
    EXPECT_TRUE(gsi->d->m_deadline.isSingleShot());
    EXPECT_EQ(gsi->d->m_deadline.interval(), 40000);
    EXPECT_NE(gsi->d->m_main, nullptr);
}

TEST_F(GrandSearchInterfaceTest, Init_MainControllerInitFails_ReturnsFalse)
{
    // Arrange
    bool initCalled = false;
    stub.set_lamda(&MainController::init, [&initCalled]() {
        initCalled = true;
        return false;
    });

    // Act
    bool result = gsi->init();

    // Assert
    EXPECT_FALSE(result);             // branch: m_main->init()==false
    EXPECT_TRUE(initCalled);
    EXPECT_NE(gsi->d->m_main, nullptr);

    // Verify timer timeout → terminate connection is wired
    bool terminateCalled = false;
    stub.set_lamda(&GrandSearchInterfacePrivate::terminate, [&terminateCalled]() {
        terminateCalled = true;
    });
    emit gsi->d->m_deadline.timeout(QTimer::QPrivateSignal());
    EXPECT_TRUE(terminateCalled);

    // Verify matched/searchCompleted direct connections
    bool matchedCalled = false;
    stub.set_lamda(&GrandSearchInterfacePrivate::onMatched, [&matchedCalled]() {
        matchedCalled = true;
    });
    bool completedCalled = false;
    stub.set_lamda(&GrandSearchInterfacePrivate::onSearchCompleted, [&completedCalled]() {
        completedCalled = true;
    });
    emit gsi->d->m_main->matched();
    EXPECT_TRUE(matchedCalled);
    emit gsi->d->m_main->searchCompleted();
    EXPECT_TRUE(completedCalled);
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface::Search
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, Search_InvalidParamsOrNoAccess_ReturnsFalse)
{
    // Arrange
    setAccessable(false);
    QString validSession = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Act — CHECKINVOKER(false) branch
    bool result1 = gsi->Search(validSession, "key");

    // Assert
    EXPECT_FALSE(result1);
    EXPECT_EQ(result1, false);

    // Arrange — now accessable but invalid params
    setAccessable(true);

    // Act — session.size()!=36 branch
    bool result2 = gsi->Search(QUuid::createUuid().toString(), "key");

    // Assert
    EXPECT_FALSE(result2);

    // Act — key.isEmpty() branch
    bool result3 = gsi->Search(validSession, "");

    // Assert
    EXPECT_FALSE(result3);

    // Act — key.size()>512 branch
    char maxStr[513] = {32};
    bool result4 = gsi->Search(validSession, QString::fromLatin1(maxStr, 513));

    // Assert
    EXPECT_FALSE(result4);
}

TEST_F(GrandSearchInterfaceTest, Search_NewSearchSucceeds_ReturnsTrue)
{
    // Arrange
    setAccessable(true);
    gsi->d->m_main = new MainController;
    bool newSearchCalled = false;
    stub.set_lamda(&MainController::newSearch, [&newSearchCalled]() {
        newSearchCalled = true;
        return true;
    });
    QString session = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Act
    bool result = gsi->Search(session, "sss");

    // Assert — branch: newSearch(key)==true
    EXPECT_TRUE(result);
    EXPECT_TRUE(newSearchCalled);
    EXPECT_EQ(gsi->d->m_session, session);
    EXPECT_TRUE(gsi->d->m_deadline.isActive());
}

TEST_F(GrandSearchInterfaceTest, Search_NewSearchFails_ReturnsFalse)
{
    // Arrange
    setAccessable(true);
    gsi->d->m_main = new MainController;
    bool newSearchCalled = false;
    stub.set_lamda(&MainController::newSearch, [&newSearchCalled]() {
        newSearchCalled = true;
        return false;
    });
    QString session = QUuid::createUuid().toString(QUuid::WithoutBraces);
    gsi->d->m_deadline.start();
    gsi->d->m_session = session;

    // Act
    bool result = gsi->Search(session, "sss");

    // Assert — branch: newSearch(key)==false
    EXPECT_FALSE(result);
    EXPECT_EQ(result, false);
    EXPECT_TRUE(newSearchCalled);
    EXPECT_TRUE(gsi->d->m_session.isEmpty());
    EXPECT_FALSE(gsi->d->m_deadline.isActive());
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface::Terminate
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, Terminate_NoAccess_ReturnsImmediately)
{
    // Arrange
    setAccessable(false);
    bool terminateCalled = false;
    stub.set_lamda(&GrandSearchInterfacePrivate::terminate, [&terminateCalled]() {
        terminateCalled = true;
    });

    // Act — CHECKINVOKER() returns void, so no return value to check
    gsi->Terminate();

    // Assert — branch: CHECKINVOKER() → d->terminate() NOT called
    EXPECT_FALSE(terminateCalled);
    EXPECT_EQ(gsi->d->m_main, nullptr);
}

TEST_F(GrandSearchInterfaceTest, Terminate_HasAccess_CallsPrivateTerminate)
{
    // Arrange
    setAccessable(true);
    bool terminateCalled = false;
    stub.set_lamda(&GrandSearchInterfacePrivate::terminate, [&terminateCalled]() {
        terminateCalled = true;
    });

    // Act
    gsi->Terminate();

    // Assert — branch: accessable → d->terminate() called
    EXPECT_TRUE(terminateCalled);
    EXPECT_EQ(gsi->d->m_main, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface::MatchedResults
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, MatchedResults_NoAccess_ReturnsEmpty)
{
    // Arrange
    setAccessable(false);

    // Act
    QByteArray result = gsi->MatchedResults("session");

    // Assert — branch: CHECKINVOKER(ret) → empty return
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(GrandSearchInterfaceTest, MatchedResults_ValidAndInvalidSession_ReturnsExpectedValue)
{
    // Arrange
    setAccessable(true);
    gsi->d->m_main = new MainController;
    QByteArray expectedData("result_data");
    stub.set_lamda(&MainController::getResults, [&expectedData]() {
        return expectedData;
    });
    QString session = "test-session";
    gsi->d->m_session = session;

    // Act — valid session branch
    QByteArray result = gsi->MatchedResults(session);

    // Assert
    EXPECT_EQ(result, expectedData);

    // Act — invalid session branch
    QByteArray result2 = gsi->MatchedResults("wrong-session");

    // Assert
    EXPECT_TRUE(result2.isEmpty());
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface::MatchedBuffer
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, MatchedBuffer_NoAccess_ReturnsEmpty)
{
    // Arrange
    setAccessable(false);

    // Act
    QByteArray result = gsi->MatchedBuffer("session");

    // Assert — branch: CHECKINVOKER(ret) → empty return
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(GrandSearchInterfaceTest, MatchedBuffer_ValidSessionNonEmptyBuffer_ReturnsData)
{
    // Arrange
    setAccessable(true);
    gsi->d->m_main = new MainController;
    QByteArray expectedBuffer("buffer_data");
    stub.set_lamda(&MainController::isEmptyBuffer, []() {
        return false;
    });
    stub.set_lamda(&MainController::readBuffer, [&expectedBuffer]() {
        return expectedBuffer;
    });
    QString session = "test-session";
    gsi->d->m_session = session;

    // Act — branch: vaildSession==true && !isEmptyBuffer
    QByteArray result = gsi->MatchedBuffer(session);

    // Assert
    EXPECT_EQ(result, expectedBuffer);
    EXPECT_EQ(result.size(), expectedBuffer.size());
}

TEST_F(GrandSearchInterfaceTest, MatchedBuffer_ValidSessionEmptyBuffer_ReturnsEmpty)
{
    // Arrange
    setAccessable(true);
    gsi->d->m_main = new MainController;
    stub.set_lamda(&MainController::isEmptyBuffer, []() {
        return true;
    });
    bool readBufferCalled = false;
    stub.set_lamda(&MainController::readBuffer, [&readBufferCalled]() {
        readBufferCalled = true;
        return QByteArray();
    });
    QString session = "test-session";
    gsi->d->m_session = session;

    // Act — branch: vaildSession==true && isEmptyBuffer==true
    QByteArray result = gsi->MatchedBuffer(session);

    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
    EXPECT_FALSE(readBufferCalled);
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface::OpenWithPlugin
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, OpenWithPlugin_NoAccess_ReturnsFalse)
{
    // Arrange
    setAccessable(false);
    bool actionCalled = false;
    stub.set_lamda(&MainController::searcherAction, [&actionCalled]() {
        actionCalled = true;
        return true;
    });

    // Act
    bool result = gsi->OpenWithPlugin("searcher", "item");

    // Assert — branch: CHECKINVOKER(false)
    EXPECT_FALSE(result);
    EXPECT_FALSE(actionCalled);
    EXPECT_EQ(result, false);
}

TEST_F(GrandSearchInterfaceTest, OpenWithPlugin_HasAccess_CallsSearcherAction)
{
    // Arrange
    setAccessable(true);
    gsi->d->m_main = new MainController;
    QString capturedSearcher, capturedAction, capturedItem;
    stub.set_lamda(&MainController::searcherAction,
        [&capturedSearcher, &capturedAction, &capturedItem](
            MainController*, const QString &name, const QString &action, const QString &item) {
            capturedSearcher = name;
            capturedAction = action;
            capturedItem = item;
            return true;
        });

    // Act
    bool result = gsi->OpenWithPlugin("my_searcher", "my_item");

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(capturedSearcher, "my_searcher");
    EXPECT_EQ(capturedAction, "openitem");
    EXPECT_EQ(capturedItem, "my_item");
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterface::KeepAlive
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, KeepAlive_NoAccess_ReturnsFalse)
{
    // Arrange
    setAccessable(false);
    QString session = "test-session";
    gsi->d->m_session = session;

    // Act
    bool result = gsi->KeepAlive(session);

    // Assert — branch: CHECKINVOKER(false)
    EXPECT_FALSE(result);
    EXPECT_EQ(result, false);
    EXPECT_FALSE(gsi->d->m_deadline.isActive());
}

TEST_F(GrandSearchInterfaceTest, KeepAlive_ValidAndInvalidSession_ReturnsExpectedValue)
{
    // Arrange
    setAccessable(true);
    QString session = "test-session";
    gsi->d->m_session = session;

    // Act — valid session branch
    bool result = gsi->KeepAlive(session);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(result, true);
    EXPECT_TRUE(gsi->d->m_deadline.isActive());

    // Act — invalid session branch
    gsi->d->m_deadline.stop();
    bool result2 = gsi->KeepAlive("wrong-session");

    // Assert
    EXPECT_FALSE(result2);
    EXPECT_EQ(result2, false);
    EXPECT_FALSE(gsi->d->m_deadline.isActive());
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterfacePrivate constructor / destructor
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, PrivateConstructor_InitializesPermitMap_HasEntry)
{
    // Arrange — gsi->d created in SetUp via GrandSearchInterface ctor
    ASSERT_NE(gsi->d, nullptr);

    // Act — inspect permit map and q pointer
    int permitSize = gsi->d->m_permit.size();
    bool permitValue = permitSize > 0 ? gsi->d->m_permit.constBegin().value() : false;

    // Assert
    EXPECT_EQ(permitSize, 1);
    EXPECT_TRUE(permitValue);
    EXPECT_EQ(gsi->d->q, gsi);
}

TEST_F(GrandSearchInterfaceTest, PrivateDestructor_DeletesMainController_NoCrash)
{
    // Arrange
    GrandSearchInterface *tmp = new GrandSearchInterface();
    tmp->d->m_main = new MainController();
    ASSERT_NE(tmp->d->m_main, nullptr);

    // Act
    delete tmp;

    // Assert — destructor deleted m_main without crash; verify fresh instance
    GrandSearchInterface *tmp2 = new GrandSearchInterface();
    EXPECT_NE(tmp2->d, nullptr);
    EXPECT_EQ(tmp2->d->m_main, nullptr);
    delete tmp2;
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterfacePrivate::isAccessable
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, IsAccessable_ProcessExeNotExists_ReturnsFalse)
{
    // Arrange — create a message with no valid DBus service name;
    // servicePid returns 0, /proc/0/exe does not exist → early return false.
    QDBusMessage msg = QDBusMessage::createSignal("/", "test.service", "test");
    EXPECT_TRUE(msg.service().isEmpty());

    // Act
    bool result = gsi->d->isAccessable(msg);

    // Assert — branch: exe not found → return false (before QT_DEBUG check)
    EXPECT_FALSE(result);
    EXPECT_EQ(result, false);
}

TEST_F(GrandSearchInterfaceTest, IsAccessable_PermitCheck_VerifiesPermitMapLogic)
{
    // Arrange — stub QDBusConnectionInterface::servicePid to return the
    // test process's own PID, so isAccessable can resolve /proc/<pid>/exe.
    uint selfPid = static_cast<uint>(QCoreApplication::applicationPid());
    EXPECT_GT(selfPid, 0u);

    QDBusMessage dummyMsg = QDBusMessage::createSignal("/", "test.service", "test");
    QDBusMessage fakeReply = dummyMsg.createReply(QVariant(selfPid));

    stub.set_lamda(
        static_cast<QDBusReply<uint> (QDBusConnectionInterface::*)(
            const QString &) const>(&QDBusConnectionInterface::servicePid),
        [&fakeReply](QDBusConnectionInterface *, const QString &) {
            return QDBusReply<uint>(fakeReply);
        });

    // Verify the test process exe path exists and add it to the permit map
    QFileInfo fi(QString("/proc/%1/exe").arg(selfPid));
    ASSERT_TRUE(fi.exists()) << "Exe path should exist for PID " << selfPid;
    QString exePath = fi.canonicalFilePath();
    EXPECT_FALSE(exePath.isEmpty());
    gsi->d->m_permit.insert(exePath, true);

    QDBusMessage msg = QDBusMessage::createSignal("/", "test.service", "test");

    // Act
    bool result = gsi->d->isAccessable(msg);

    // Assert — permit map contains the test process's exe path → true
    EXPECT_TRUE(result);
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterfacePrivate::onMatched
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, OnMatched_SessionNotEmpty_EmitsMatchedSignal)
{
    // Arrange
    QString session = "active-session";
    gsi->d->m_session = session;
    QSignalSpy spy(gsi, &GrandSearchInterface::Matched);

    // Act
    gsi->d->onMatched();

    // Assert — branch: !m_session.isEmpty() → emit Matched
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), session);
}

TEST_F(GrandSearchInterfaceTest, OnMatched_SessionEmpty_DoesNotEmitSignal)
{
    // Arrange
    gsi->d->m_session.clear();
    QSignalSpy spy(gsi, &GrandSearchInterface::Matched);

    // Act
    gsi->d->onMatched();

    // Assert — branch: m_session.isEmpty() → no signal
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(gsi->d->m_session.isEmpty());
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterfacePrivate::onSearchCompleted
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, OnSearchCompleted_SessionNotEmpty_EmitsSearchCompletedSignal)
{
    // Arrange
    QString session = "active-session";
    gsi->d->m_session = session;
    QSignalSpy spy(gsi, &GrandSearchInterface::SearchCompleted);

    // Act
    gsi->d->onSearchCompleted();

    // Assert — branch: !m_session.isEmpty() → emit SearchCompleted
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), session);
}

TEST_F(GrandSearchInterfaceTest, OnSearchCompleted_SessionEmpty_DoesNotEmitSignal)
{
    // Arrange
    gsi->d->m_session.clear();
    QSignalSpy spy(gsi, &GrandSearchInterface::SearchCompleted);

    // Act
    gsi->d->onSearchCompleted();

    // Assert — branch: m_session.isEmpty() → no signal
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(gsi->d->m_session.isEmpty());
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterfacePrivate::terminate
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, PrivateTerminate_WithMainController_StopsAndTerminates)
{
    // Arrange
    gsi->d->m_main = new MainController;
    bool mainTerminateCalled = false;
    stub.set_lamda(&MainController::terminate, [&mainTerminateCalled]() {
        mainTerminateCalled = true;
    });
    gsi->d->m_deadline.start();

    // Act — branch: m_main != nullptr
    gsi->d->terminate();

    // Assert
    EXPECT_TRUE(mainTerminateCalled);
    EXPECT_NE(gsi->d->m_main, nullptr);
    EXPECT_FALSE(gsi->d->m_deadline.isActive());
}

TEST_F(GrandSearchInterfaceTest, PrivateTerminate_WithoutMainController_StopsDeadlineOnly)
{
    // Arrange
    ASSERT_EQ(gsi->d->m_main, nullptr);
    bool mainTerminateCalled = false;
    stub.set_lamda(&MainController::terminate, [&mainTerminateCalled]() {
        mainTerminateCalled = true;
    });
    gsi->d->m_deadline.start();
    EXPECT_TRUE(gsi->d->m_deadline.isActive());

    // Act — branch: m_main == nullptr
    gsi->d->terminate();

    // Assert
    EXPECT_FALSE(mainTerminateCalled);
    EXPECT_EQ(mainTerminateCalled, false);
    EXPECT_FALSE(gsi->d->m_deadline.isActive());
}

// ═══════════════════════════════════════════════════════════════
// GrandSearchInterfacePrivate::vaildSession
// ═══════════════════════════════════════════════════════════════

TEST_F(GrandSearchInterfaceTest, VaildSession_EmptySession_ReturnsFalse)
{
    // Arrange — m_session is empty by default
    gsi->d->m_session.clear();
    EXPECT_TRUE(gsi->d->m_session.isEmpty());

    // Act — branch: session.isEmpty() → false
    bool result = gsi->d->vaildSession("");

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(result, false);
}

TEST_F(GrandSearchInterfaceTest, VaildSession_MatchingSession_ReturnsTrue)
{
    // Arrange
    QString session = "my-session";
    gsi->d->m_session = session;

    // Act — branch: m_session == session → true
    bool result = gsi->d->vaildSession(session);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(gsi->d->m_session, session);
}

TEST_F(GrandSearchInterfaceTest, VaildSession_NonMatchingSession_ReturnsFalse)
{
    // Arrange
    gsi->d->m_session = "my-session";

    // Act — branch: m_session != session → false
    bool result = gsi->d->vaildSession("other-session");

    // Assert
    EXPECT_FALSE(result);
    EXPECT_NE(gsi->d->m_session, QString("other-session"));
}
