// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// MainController 桩实现——提供最小链接实现，避免编译 maincontroller.cpp 拉入深层依赖链。
// stub-ext 的 set_lamda 在运行时替换这些函数地址，桩实现仅满足链接器需求。

#include "maincontroller/maincontroller.h"

#include <QByteArray>

using namespace GrandSearch;

MainController::MainController(QObject *parent)
    : QObject(parent)
{
}

bool MainController::init()
{
    return false;
}

bool MainController::newSearch(const QString &key)
{
    Q_UNUSED(key)
    return false;
}

void MainController::terminate()
{
}

QByteArray MainController::getResults() const
{
    return {};
}

QByteArray MainController::readBuffer() const
{
    return {};
}

bool MainController::isEmptyBuffer() const
{
    return true;
}

bool MainController::searcherAction(const QString &name, const QString &action, const QString &item)
{
    Q_UNUSED(name)
    Q_UNUSED(action)
    Q_UNUSED(item)
    return false;
}

