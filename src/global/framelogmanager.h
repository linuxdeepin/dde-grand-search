// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRAMELOGMANAGER_H
#define FRAMELOGMANAGER_H

#include <QObject>

namespace Dtk {
namespace Core {
class Logger;
}
}

namespace GrandSearch {

class FrameLogManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FrameLogManager)

public:
    static FrameLogManager *instance();
    void applySuggestedLogSettings();
    Dtk::Core::Logger *globalDtkLogger();

private:
    explicit FrameLogManager(QObject *parent = nullptr);
    ~FrameLogManager();
};

}   // namespace GrandSearch

#define dgsLogManager ::GrandSearch::FrameLogManager::instance()

#endif   // FRAMELOGMANAGER_H
