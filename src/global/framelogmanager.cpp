// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "framelogmanager.h"

#include <DLog>

using namespace GrandSearch;
DCORE_USE_NAMESPACE

FrameLogManager::FrameLogManager(QObject *parent)
    : QObject(parent)
{
}

FrameLogManager::~FrameLogManager()
{
}

FrameLogManager *FrameLogManager::instance()
{
    static FrameLogManager ins;
    return &ins;
}

void FrameLogManager::applySuggestedLogSettings()
{
// DtkCore 5.6.8版本，支持journal方式日志存储，
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    DLogManager::registerJournalAppender();   // 开启journal日志存储
#ifdef QT_DEBUG
    DLogManager::registerConsoleAppender();   // Release下，标准输出需要禁止，否则会导致一系列问题
#endif
// 为保证兼容性，在该版本以下，采用原有log文件日志输出方式保存日志
#else
    // 设置终端和文件记录日志
    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    DLogManager::setLogFormat(logFormat);
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
#endif
}

Dtk::Core::Logger *FrameLogManager::globalDtkLogger()
{
    return DTK_CORE_NAMESPACE::Logger::globalInstance();
}
