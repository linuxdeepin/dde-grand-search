// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "embeddingpluginwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static constexpr char PLUGINSNAME[] = "uos-ai-rag";

EmbeddingPluginWidget::EmbeddingPluginWidget(QWidget *parent) : DWidget(parent)
{
    initUI();
}

void EmbeddingPluginWidget::setText(const QString &theme, const QString &summary)
{
    m_pLabelTheme->setText(theme);
    m_pLabelSummary->setText(summary);
}

bool EmbeddingPluginWidget::isInstalled()
{
    return m_pManageModel->property("modelStatus").toInt() != Uninstall;
}

void EmbeddingPluginWidget::checkInstallStatus()
{
    QProcess m_pProcess;
    m_pProcess.start("dpkg-query", QStringList() << "-W" << QString("-f='${db:Status-Status}\n'") << PLUGINSNAME);
    m_pProcess.waitForFinished();
    QByteArray reply = m_pProcess.readAllStandardOutput();
    bool installStatus = (reply == "'installed\n'" ? true : false);
    m_pManageModel->setProperty("modelStatus", installStatus ? Install : Uninstall);
    m_pManageModel->setVisible(!installStatus);

    m_pLabelStatus->setText(installStatus ? tr("Installed") : tr("Not Installed"));

    emit pluginStateChanged(installStatus);
}

void EmbeddingPluginWidget::paintEvent(QPaintEvent *e)
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
    if (m_pManageModel->property("modelStatus").toInt() == Install) {
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

void EmbeddingPluginWidget::initUI()
{
    m_pLabelTheme = new DLabel;
    DFontSizeManager::instance()->bind(m_pLabelTheme, DFontSizeManager::T6, QFont::Medium);
    m_pLabelTheme->setElideMode(Qt::ElideRight);

    m_pLabelStatus = new DLabel(tr("Not Installed"));
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

    m_pManageModel = new ModelManageButton(tr("Install"), this);
    QPixmap pixmap = QApplication::style()->standardIcon(QStyle::SP_ArrowDown).pixmap(QSize(10, 10));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), DGuiApplicationHelper::instance()->applicationPalette().textTips());
    m_pManageModel->setIcon(pixmap);
    m_pManageModel->setProperty("modelStatus", Uninstall);
    DFontSizeManager::instance()->bind(m_pManageModel, DFontSizeManager::T8, QFont::Medium);
    m_pManageModel->updateRectSize();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 8, 10, 6);
    mainLayout->setSpacing(5);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_pLabelSummary, 0, Qt::AlignLeft);
    mainLayout->addWidget(m_pManageModel, 0, Qt::AlignLeft);

    setLayout(mainLayout);

    connect(m_pManageModel, &ModelManageButton::clicked, this, &EmbeddingPluginWidget::openAppStore);
}

void EmbeddingPluginWidget::openAppStore()
{
    qInfo() << "open app store" << PLUGINSNAME;
    QDBusMessage msg = QDBusMessage::createMethodCall("com.home.appstore.client", "/com/home/appstore/client",
                                                      "com.home.appstore.client", "openBusinessUri");
    QVariantList list;
    list.append(QString("app_detail_info/%0").arg(PLUGINSNAME));
    msg.setArguments(list);

    QDBusConnection::sessionBus().asyncCall(msg, 100);
    return;
}
