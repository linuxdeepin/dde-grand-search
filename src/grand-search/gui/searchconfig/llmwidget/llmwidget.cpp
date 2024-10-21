// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llmwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QtConcurrent>
#include <QStandardPaths>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DMenu>
#include <DToolButton>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static constexpr int TIMERBEGIN = 720;//5秒轮询安装和更新状态60分钟

static constexpr char MODELNAME[] = "/yourong1.5B-Instruct-GGUF";

LLMWidget::LLMWidget(DWidget *parent)
    : DWidget(parent),
      m_pProcess(new QProcess)
{
    initUI();
    initConnect();
    installEventFilter(this);
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    m_installPath = homePath + "/.local/share/deepin-modelhub/models";
}

LLMWidget::~LLMWidget()
{
    if (m_pProcess) {
        m_pProcess->terminate();
        m_pProcess->deleteLater();
    }
}

void LLMWidget::initUI()
{
    m_pLabelTheme = new DLabel;
    DFontSizeManager::instance()->bind(m_pLabelTheme, DFontSizeManager::T6, QFont::Medium);
    m_pLabelTheme->setElideMode(Qt::ElideRight);

    m_pLabelStatus = new DLabel(tr("NotInstalled"));
    m_pLabelStatus->setForegroundRole(QPalette::Text);
    DFontSizeManager::instance()->bind(m_pLabelStatus, DFontSizeManager::T8, QFont::Medium);

    auto topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(m_pLabelTheme, 0, Qt::AlignLeft);
    topLayout->addStretch();
    topLayout->addWidget(m_pLabelStatus, 0, Qt::AlignRight);


    m_pLabelSummary = new DLabel;
    m_pLabelSummary->setForegroundRole(QPalette::Text);
    DFontSizeManager::instance()->bind(m_pLabelSummary, DFontSizeManager::T8, QFont::Normal);
    m_pLabelSummary->setElideMode(Qt::ElideRight);

    m_pManageModel = new ModelManageButton(tr("Install Model"), this);
    QPixmap pixmap = QApplication::style()->standardIcon(QStyle::SP_ArrowDown).pixmap(QSize(10, 10));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), DGuiApplicationHelper::instance()->applicationPalette().textTips());
    m_pManageModel->setIcon(pixmap);
    m_pManageModel->setProperty("modelStatus", Uninstall);
    DFontSizeManager::instance()->bind(m_pManageModel, DFontSizeManager::T8, QFont::Medium);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 8, 10, 6);
    mainLayout->setSpacing(5);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_pLabelSummary, 0, Qt::AlignLeft);
    mainLayout->addWidget(m_pManageModel, 0, Qt::AlignLeft);

    m_updateAction = new QAction(tr("Update model"));
    m_uninstallAction = new QAction(tr("Uninstall model"));

    m_pMenu = new DMenu(this);
    m_pMenu->setMinimumWidth(92);
    m_pMenu->installEventFilter(this);
    m_pMenu->addAction(m_updateAction);
    m_pMenu->addAction(m_uninstallAction);

    setLayout(mainLayout);
}

void LLMWidget::initConnect()
{
    connect(m_pManageModel, &DCommandLinkButton::clicked, this, &LLMWidget::onClickedStatusBtn);
    connect(m_pMenu, &QMenu::triggered, this, &LLMWidget::onMoreMenuTriggered);

    connect(m_pProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));
}

void LLMWidget::paintEvent(QPaintEvent* e)
{
    DPalette pl = m_pLabelSummary->palette();
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().color(DPalette::BrightText);
    if (!isActiveWindow())
        color.setAlphaF(0.4);
    else
        color.setAlphaF(0.6);
    pl.setColor(QPalette::Text, color);
    m_pLabelSummary->setPalette(pl);

    pl = m_pLabelStatus->palette();
    if (m_pManageModel->property("modelStatus").toInt() != Uninstall) {
        color = QColor(21, 190, 76);
        color.setAlphaF(1);
    } else
        color.setAlphaF(0.6);

    if (!isActiveWindow())
        color.setAlphaF(0.4);

    pl.setColor(QPalette::Text, color);
    m_pLabelStatus->setPalette(pl);

    DWidget::paintEvent(e);
}

void LLMWidget::onClickedStatusBtn()
{
    int modelStatus = m_pManageModel->property("modelStatus").toInt();
    switch (modelStatus) {
    case Install:
        onUninstall();
        break;
    case Uninstall:
        onInstall();
        break;
    case InstallAndUpdate:
        m_pMenu->exec(m_pManageModel->mapToGlobal(m_pManageModel->rect().bottomLeft() + QPoint(0, 5)));
        break;
    default:
        break;
    }
}

void LLMWidget::onMoreMenuTriggered(const QAction *action)
{
    if (action == m_uninstallAction) {
        onUninstall();
    } else {
//        onInstall();//更新模型也打开应用商店的下载界面去更新
    }
}

void LLMWidget::onInstall()
{
    if (!m_pProcess->atEnd()) return;
    if (m_pProcess->state() == QProcess::Running)
        m_pProcess->waitForFinished();

    QString program = "git";
    QStringList arguments;
    arguments << "clone" << "https://www.modelscope.cn/uniontech-yourong/yourong1.5B-Instruct-GGUF.git";

    m_pProcess->setWorkingDirectory(m_installPath);
    m_pProcess->start(program, arguments);

    onDealInstalledModel();
}

void LLMWidget::onUninstall()
{
    QDir installFile(m_installPath + MODELNAME);

    if (installFile.exists()) {
        if (installFile.removeRecursively())
            qInfo() << "Directory removed successfully.";
        else
            qWarning() << "Failed to remove directory.";
    }
    checkInstallStatus();

}

void LLMWidget::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (process) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            qInfo() << "Git clone successful.";
            if (onDealInstalledModel())
                checkInstallStatus();
        } else {
            qWarning() << "Git clone failed.";
        }
    }
}

bool LLMWidget::onDealInstalledModel()
{
    QDir installFile(m_installPath + MODELNAME);

    if (installFile.exists()) {
        QDir sourceDir(m_installPath + MODELNAME + "/modelhub");
        if (!sourceDir.exists()) {
            qWarning() << "Source directory does not exist.";
            return false;
        }

        QDir destinationDir(m_installPath + MODELNAME);

        if (destinationDir.mkpath("gguf")) {
            qInfo() << "Directory created successfully:";
        } else {
            qWarning() << "Failed to create directory:" ;
            return false;
        }

        destinationDir.cd("gguf");

        QStringList files = sourceDir.entryList(QDir::Files);
        for (const QString &fileName : files) {
            QString sourceFilePath = sourceDir.absoluteFilePath(fileName);
            QString destinationFilePath = destinationDir.absoluteFilePath(fileName);
            if (!QFile::rename(sourceFilePath, destinationFilePath)) {
                qWarning() << "Failed to move file" << sourceFilePath << "to" << destinationFilePath;
                return false;
            }
        }

        auto filesInfo = installFile.entryInfoList(QDir::Files);
        for (const QFileInfo &info : filesInfo) {
            if (info.suffix() == "gguf") {
                QString sourceFilePath = info.absoluteFilePath();
                if (!QFile::rename(sourceFilePath, destinationDir.absoluteFilePath(info.fileName()))) {
                    qWarning() << "Failed to move file:";
                    return false;
                }
            }
        }
    }
    return true;
}

void LLMWidget::checkInstallStatus()
{
    QDir installFile(m_installPath + MODELNAME);

    m_pManageModel->setProperty("modelStatus", installFile.exists() ? Install : Uninstall);

    changeInstallStatus();
}

void LLMWidget::onCloseEvent()
{
    if (m_pProcess->state() == QProcess::Running && Uninstall == m_pManageModel->property("modelStatus").toInt())
        onUninstall();
}

void LLMWidget::changeInstallStatus()
{
    int modelStatus = m_pManageModel->property("modelStatus").toInt();
    switch (modelStatus) {
    case Install: {
        m_pManageModel->setText(tr("UnInstall Model"));
        m_pLabelStatus->setText(tr("Installed"));
        break;
        }
    case Uninstall: {
        m_pManageModel->setText(tr("Install Model"));
        m_pLabelStatus->setText(tr("NotInstalled"));
        break;
        }
    default:
        break;
    }
}

void LLMWidget::setText(const QString &theme, const QString &summary)
{
    m_pLabelTheme->setText(theme);
    m_pLabelSummary->setText(summary);
}
