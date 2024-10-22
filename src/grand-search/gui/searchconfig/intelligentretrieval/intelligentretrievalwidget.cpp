// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intelligentretrievalwidget.h"
#include "detailcheckbox.h"
#include "autoindexstatus.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "gui/searchconfig/llmwidget/llmwidget.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"
#include "global/builtinsearch.h"
#include "gui/searchconfig/tipslabel.h"

#include <DFontSizeManager>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

#define INER_APP_NAME "dde-grand-search"
static QDBusMessage createQueryLangMsg(const QString &method) {
    QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.ai.daemon.QueryLang", "/org/deepin/ai/daemon/QueryLang",
                                                      "org.deepin.ai.daemon.QueryLang", method);
    return msg;
}

#ifdef VECTOR_SEARCH
static QDBusMessage createVectorMsg(const QString &method) {
    QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.ai.daemon.VectorIndex", "/org/deepin/ai/daemon/VectorIndex",
                                                      "org.deepin.ai.daemon.VectorIndex", method);
    return msg;
}
#endif

IntelligentRetrievalWidget::IntelligentRetrievalWidget(QWidget *parent)
    : DWidget(parent)
{

    m_mainLayout = new QVBoxLayout();
    setLayout(m_mainLayout);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(1);

    m_semantic = new SwitchWidget(tr("AI Smart Search"), this);
    m_semantic->setIconEnable(false);
    m_semantic->setMinimumSize(CHECKBOXITEMWIDTH, DETAILCHECKBOXITEMHEIGHT);
    bool turnOn = false;
    {
        auto cfg = SearchConfig::instance();
        turnOn = cfg->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, false).toBool();
        m_semantic->setChecked(turnOn);
        m_semantic->layout()->setContentsMargins(0, 0, 10, 0);
        m_semantic->setFixedHeight(SWITCHWIDGETHEIGHT);
    }
    m_mainLayout->addWidget(m_semantic);

    m_detailLabel = new TipsLabel(tr("When turned on, you can try to search for local documents using natural language descriptions, such as \"last week's documents\".")
                               , this);
    m_detailLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_detailLabel);

    m_indexWidget = new QWidget(this);
    m_mainLayout->addWidget(m_indexWidget);
    m_indexWidget->setVisible(turnOn);

    m_indexLayout = new QVBoxLayout();
    m_indexLayout->setContentsMargins(0, 0, 0, 0);
    m_indexLayout->setSpacing(0);
    m_indexLayout->addSpacing(10);
    m_indexWidget->setLayout(m_indexLayout);

    // feat
    {
        m_featIndex = new SwitchWidget(tr("Automatic index update"), m_indexWidget);
        m_featIndex->setEnableBackground(true);
        m_featIndex->setFixedSize(SWITCHWIDGETWIDTH, DETAILSWITCHWIDGETHEIGHT / 2);
        m_featIndex->setIconEnable(false);

        m_indexLayout->addWidget(m_featIndex);
        m_indexLayout->addSpacing(10);

        m_indexStatus = new AutoIndexStatus(m_indexWidget);
        m_indexStatus->setMinimumSize(SWITCHWIDGETWIDTH, DETAILSWITCHWIDGETHEIGHT / 2);
        m_indexLayout->addWidget(m_indexStatus);
        m_indexStatus->setVisible(false);
    }

    // model
    {
        RoundedBackground *bkg = new RoundedBackground(m_indexWidget);
        bkg->setMinimumSize(CHECKBOXITEMWIDTH, SWITCHWIDGETHEIGHT);
        bkg->setTopRound(true);
        bkg->setBottomRound(true);

        QVBoxLayout *vl = new QVBoxLayout(bkg);
        bkg->setLayout(vl);
        vl->setContentsMargins(0, 2, 0, 5);
        m_llmWidget = new LLMWidget(bkg);
        m_llmWidget->setText(tr("UOS AI LLM"),tr("After installing the UOS AI large model, you can use the AI intelligent search function without an internet connection."));
        m_llmWidget->checkInstallStatus();
        vl->addWidget(m_llmWidget);
        m_indexLayout->addWidget(bkg);
    }

    // full text
    {
        m_indexLayout->addSpacing(10);
        m_fullTextIndex = new SwitchWidget(tr("Full Text Search"), m_indexWidget);
        m_fullTextIndex->setEnableBackground(true);
        m_fullTextIndex->setFixedSize(SWITCHWIDGETWIDTH, DETAILSWITCHWIDGETHEIGHT / 2);
        m_fullTextIndex->setIconEnable(false);
        m_indexLayout->addWidget(m_fullTextIndex);

        m_indexLayout->addSpacing(5);
        m_fullTextLabel = new TipsLabel(tr("When turned on, full text search can be used in the file manager and grand search.")
                                   , this);
        auto margin = m_fullTextLabel->contentsMargins();
        margin.setLeft(10);
        m_fullTextLabel->setContentsMargins(margin);
        m_indexLayout->addWidget(m_fullTextLabel);
    }

#ifdef VECTOR_SEARCH
    // 超链接
    m_vectorDetail = new QLabel;
    m_vectorDetail->setContextMenuPolicy(Qt::NoContextMenu);
    connect(m_vectorDetail, &QLabel::linkActivated, this, &IntelligentRetrievalWidget::openAppStore);

    m_vector = new DetailCheckBox(tr("Intelligent full text search"), m_vectorDetail, this);
    m_vector->setMinimumSize(CHECKBOXITEMWIDTH, DETAILCHECKBOXITEMHEIGHT);
    m_vector->setBottomRound(true);
    m_mainLayout->addWidget(m_vector);
#endif

    updateState();

    connect(&m_timer, &QTimer::timeout, this, &IntelligentRetrievalWidget::updateState);
    m_timer.start(5000);
    connect(m_semantic, &SwitchWidget::checkedChanged, this, &IntelligentRetrievalWidget::checkChanged);
#ifdef VECTOR_SEARCH
    connect(m_vector->checkBox(), &QCheckBox::stateChanged, this, &IntelligentRetrievalWidget::checkChanged);
#endif
    connect(m_featIndex, &SwitchWidget::checkedChanged, this, &IntelligentRetrievalWidget::checkChanged);
}

void IntelligentRetrievalWidget::updateState()
{
    m_ignoreSigal = true;

//    if (isQueryLangSupported()) {

//        m_semantic->checkBox()->setChecked(checked);
//        m_semantic->checkBox()->setEnabled(true);
//    } else {
//        m_semantic->checkBox()->setChecked(false);
//        m_semantic->checkBox()->setEnabled(false);
//    }

#ifdef VECTOR_SEARCH
    if (isVectorSupported()) {
        bool checked = cfg->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, true).toBool();
        m_vector->checkBox()->setEnabled(true);
        m_vector->checkBox()->setChecked(checked);
        m_vector->setDetail(tr("When enabled, you can search the text of articles using disjointed and incomplete keywords."));
        if (checked) {
            m_enableIndex->setEnabled(true);
            QVariantHash status;
            bool autoidx = getIndexStatus(status);
            m_enableIndex->setChecked(autoidx);

            // update index status
            if (autoidx) {
                m_indexStatus->show();
                updateStatusContent(status);
            } else {
                m_indexStatus->hide();
            }
        } else {
            m_enableIndex->setChecked(false);
            m_enableIndex->setEnabled(false);
            m_indexStatus->hide();
        }
    } else {
        m_vector->checkBox()->setChecked(false);
        m_vector->checkBox()->setEnabled(false);
        m_vector->setDetail(tr("Please install %0 in <a href=\"%0\">the app store</a> before turning on this configuration.")
                            .arg("向量化模型"));

        m_enableIndex->setChecked(false);
        m_enableIndex->setEnabled(false);
        m_indexStatus->hide();
    }
#endif
    m_ignoreSigal = false;
}

void IntelligentRetrievalWidget::openAppStore(const QString &app)
{
    qDebug() << "open app store" << app;
    QDBusMessage msg = QDBusMessage::createMethodCall("com.home.appstore.client", "/com/home/appstore/client",
                                                      "com.home.appstore.client", "openBusinessUri");
    QVariantList list;
    list.append(QString("searchApp?keyword=%0").arg(app));
    msg.setArguments(list);

    QDBusPendingReply<void> ret = QDBusConnection::sessionBus().asyncCall(msg, 100);
    ret.waitForFinished();
    if (ret.error().type() != QDBusError::NoError)
        qWarning() << "error: " << msg.service() << QDBusError::errorString(ret.error().type());

    return;
}

void IntelligentRetrievalWidget::updateStatusContent(const QVariantHash &status)
{
    bool ok = false;
    int st = status.value("completion").toInt(&ok);
    if (!ok) {
        qWarning() << "get invalid index statuas" << st;
        st = -1;
    }

    if (st == 0) {
        m_indexStatus->updateContent(AutoIndexStatus::Updating,
                    tr("Intelligent search indexing is being updated, which may take up more resources, please keep the power access."));
    } else if (st == 1) {
        auto dt = QDateTime::fromSecsSinceEpoch(status.value("updatetime").toLongLong());
        m_indexStatus->updateContent(AutoIndexStatus::Success,
                    tr("Smart Search indexing update is complete. Last update time: %0").arg(dt.toString("yyyy-MM-dd hh:mm:ss")));
    } else {
        m_indexStatus->updateContent(AutoIndexStatus::Fail,
                    tr("Indexing error!"));
    }
}

bool IntelligentRetrievalWidget::isQueryLangSupported()
{
    auto msg = createQueryLangMsg("Enable");
    QDBusPendingReply<bool> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();
    return ret.isValid() && ret;
}

#ifdef VECTOR_SEARCH
bool IntelligentRetrievalWidget::isVectorSupported()
{
    auto msg = createVectorMsg("Enable");
    QDBusPendingReply<bool> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();
    return ret.isValid() && ret;
}
#endif

void IntelligentRetrievalWidget::checkChanged()
{
    if (m_ignoreSigal)
        return;

    auto sd = sender();
    auto cfg = SearchConfig::instance();
    if (sd == m_semantic) {
        bool on = m_semantic->checked();
        cfg->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, on);
        m_indexWidget->setVisible(on);
        adjustSize();
    }
#ifdef VECTOR_SEARCH
    else if (sd == m_vector->checkBox()){
        bool on = m_vector->checkBox()->isChecked();
        cfg->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, on);
        // 关闭搜索时联动关闭索引
        if (!on)
            setAutoIndex(false);
    }
#endif
    else if (sd == m_featIndex) {
        setAutoIndex(m_featIndex->checked());
    }

    updateState();
}

void IntelligentRetrievalWidget::setAutoIndex(bool on)
{
    QDBusMessage msg = createQueryLangMsg("SetSemanticOn");
    QVariantList argvs;
    argvs.append(on);
    msg.setArguments(argvs);
    QDBusConnection::sessionBus().asyncCall(msg, 500);
}

#ifdef VECTOR_SEARCH
void IntelligentRetrievalWidget::setAutoIndex(bool on)
{
    auto msg = createVectorMsg("setAutoIndex");
    QVariantList list;
    list.append(QString(INER_APP_NAME));
    list.append(on);
    msg.setArguments(list);

    QDBusPendingReply<void> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();
    if (ret.error().type() != QDBusError::NoError) {
        qWarning() << "error: " << msg.service() << QDBusError::errorString(ret.error().type());
    }
    return ;
}

bool IntelligentRetrievalWidget::getIndexStatus(QVariantHash &statuts)
{
    auto msg = createVectorMsg("getAutoIndexStatus");
    QVariantList list;
    list.append(QString(INER_APP_NAME));
    msg.setArguments(list);

    QDBusPendingReply<QString> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();

    if (ret.error().type() != QDBusError::NoError) {
        qWarning() << "error: " << msg.service() << QDBusError::errorString(ret.error().type()) << ret;
        return false;
    }

    QString json = ret;
    statuts = QJsonDocument::fromJson(json.toUtf8()).object().toVariantHash();
    return statuts.value("enable", false).toBool();
}
#endif

void IntelligentRetrievalWidget::onCloseEvent()
{
    m_llmWidget->onCloseEvent();
}
