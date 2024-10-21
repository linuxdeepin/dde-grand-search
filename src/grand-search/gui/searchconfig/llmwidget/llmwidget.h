// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLMWIDGET_H
#define LLMWIDGET_H

#include "gui/searchconfig/roundedbackground.h"
#include "modelmanagebutton.h"
#include "dcommandlinkbutton.h"

#include <QProcess>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <DLabel>
#include <DSwitchButton>

namespace GrandSearch {

enum ModelStatus {
  None = 0,
  Install,
  Uninstall,
  InstallAndUpdate
};

class LLMWidget: public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit LLMWidget(Dtk::Widget::DWidget *parent = nullptr);
    ~LLMWidget();
    void checkInstallStatus();
//    void checkUpdateStatus();
    void setText(const QString &theme, const QString &summary);
    void onCloseEvent();

private:
    void initUI();
    void initConnect();
    void onInstall();
    void onUninstall();
    void beginTimer(const int &time);
    void checkStatusOntime();
    void changeInstallStatus();
    bool onDealInstalledModel();

private slots:
    void onClickedStatusBtn();
    void onMoreMenuTriggered(const QAction *action);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;

private:
    Dtk::Widget::DLabel *m_pLabelTheme = nullptr;
    Dtk::Widget::DLabel *m_pLabelSummary = nullptr;
    Dtk::Widget::DLabel *m_pLabelStatus = nullptr;
    ModelManageButton *m_pManageModel = nullptr;
    Dtk::Widget::DMenu *m_pMenu = nullptr;
    QAction *m_updateAction = nullptr;
    QAction *m_uninstallAction = nullptr;

    QProcess *m_pProcess = nullptr;
//    QTimer *m_timer = nullptr;
//    int m_timerCount = 0;

    QString m_installPath;
};
}

#endif // LLMWIDGET_H
