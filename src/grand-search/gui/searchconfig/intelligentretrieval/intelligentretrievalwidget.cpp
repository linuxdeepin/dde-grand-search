// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intelligentretrievalwidget.h"
#include "detailcheckbox.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"
#include "global/builtinsearch.h"
#include "gui/searchconfig/tipslabel.h"
#include <utils/report/aisearchpoint.h>
#include <utils/report/eventlogutil.h>

#include <DFontSizeManager>
#include <DDialog>
#include <DConfig>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

using namespace GrandSearch;

#define INER_APP_NAME "dde-grand-search"
static QDBusMessage createQueryLangMsg(const QString &method)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.ai.daemon.QueryLang", "/org/deepin/ai/daemon/QueryLang",
                                                      "org.deepin.ai.daemon.QueryLang", method);
    return msg;
}

#ifdef VECTOR_SEARCH
static QDBusMessage createVectorMsg(const QString &method)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.ai.daemon.VectorIndex", "/org/deepin/ai/daemon/VectorIndex",
                                                      "org.deepin.ai.daemon.VectorIndex", method);
    return msg;
}
#endif

IntelligentRetrievalWidget::IntelligentRetrievalWidget(QWidget *parent)
    : DWidget(parent)
{
    qCDebug(logGrandSearch) << "Creating IntelligentRetrievalWidget";

    m_mainLayout = new QVBoxLayout();
    setLayout(m_mainLayout);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(1);

    m_semantic = new SwitchWidget(tr("Semantic Search"), this);
    m_semantic->setIconEnable(false);
    m_semantic->setMinimumSize(CHECKBOXITEMWIDTH, DETAILCHECKBOXITEMHEIGHT);
    bool turnOn = false;
    {
        auto cfg = SearchConfig::instance();
        turnOn = cfg->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, true).toBool();
        m_semantic->setChecked(turnOn);
        m_semantic->layout()->setContentsMargins(0, 0, 10, 0);
        m_semantic->setFixedHeight(SWITCHWIDGETHEIGHT);
    }
    m_mainLayout->addWidget(m_semantic);

    m_detailLabel = new TipsLabel(tr("When turned on, you can try to search for local documents using natural language descriptions, such as \"last week's documents\"."), this);
    m_detailLabel->setWordWrap(true);
    m_detailLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
        m_ocrIndex = new SwitchWidget(tr("Ocr Text Search"), m_indexWidget);
        m_ocrIndex->setEnableBackground(true);
        m_ocrIndex->setFixedSize(SWITCHWIDGETWIDTH, DETAILSWITCHWIDGETHEIGHT / 2);
        m_ocrIndex->setIconEnable(false);
        m_indexLayout->addWidget(m_ocrIndex);
        if (turnOn) {
            // 默认状态下读取文管接口作为默认值
            DConfig *dcfg = DConfig::create("org.deepin.dde.file-manager", "org.deepin.dde.file-manager.search");
            bool isFileManagerTrue = dcfg->value("enableOcrTextSearch").toBool();
            delete dcfg;

            bool isTrue = SearchConfig::instance()->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_OCRTEXT, isFileManagerTrue).toBool();
            SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_OCRTEXT, isTrue);
            m_ocrIndex->setChecked(isTrue);
        }

        m_indexLayout->addSpacing(5);
        auto ocrTextLabel = new TipsLabel(tr("When turned on, ocr text search can be used in the file manager and grand search."), this);
        ocrTextLabel->setWordWrap(true);
        ocrTextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        auto margin = ocrTextLabel->contentsMargins();
        margin.setLeft(10);
        ocrTextLabel->setContentsMargins(margin);
        m_indexLayout->addWidget(ocrTextLabel);
    }

    // full text
    {
        m_indexLayout->addSpacing(10);
        m_fullTextIndex = new SwitchWidget(tr("Full Text Search"), m_indexWidget);
        m_fullTextIndex->setEnableBackground(true);
        m_fullTextIndex->setFixedSize(SWITCHWIDGETWIDTH, DETAILSWITCHWIDGETHEIGHT / 2);
        m_fullTextIndex->setIconEnable(false);
        m_indexLayout->addWidget(m_fullTextIndex);
        if (turnOn) {
            // 默认状态下读取文管接口作为默认值
            DConfig *dcfg = DConfig::create("org.deepin.dde.file-manager", "org.deepin.dde.file-manager.search");
            bool isFileManagerTrue = dcfg->value("enableFullTextSearch").toBool();
            delete dcfg;

            bool isTrue = SearchConfig::instance()->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, isFileManagerTrue).toBool();
            SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, isTrue);
            m_fullTextIndex->setChecked(isTrue);
        }

        m_indexLayout->addSpacing(5);
        m_fullTextLabel = new TipsLabel(tr("When turned on, full text search can be used in the file manager and grand search."), this);
        m_fullTextLabel->setWordWrap(true);
        m_fullTextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        auto margin = m_fullTextLabel->contentsMargins();
        margin.setLeft(10);
        m_fullTextLabel->setContentsMargins(margin);
        m_indexLayout->addWidget(m_fullTextLabel);
    }

    connect(m_semantic, &SwitchWidget::checkedChanged, this, &IntelligentRetrievalWidget::checkChanged);
    connect(m_ocrIndex, &SwitchWidget::checkedChanged, this, &IntelligentRetrievalWidget::checkChanged);
    connect(m_fullTextIndex, &SwitchWidget::checkedChanged, this, &IntelligentRetrievalWidget::checkChanged);

    qCDebug(logGrandSearch) << "IntelligentRetrievalWidget created - AI Search enabled:" << turnOn;
}

void IntelligentRetrievalWidget::checkChanged()
{
    auto sd = sender();
    auto cfg = SearchConfig::instance();
    if (sd == m_semantic) {
        bool on = m_semantic->checked();
        cfg->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, on);
        ReportIns()->writeEvent(report::AiSearchPoint(on).assemblingData());
        m_indexWidget->setVisible(on);
        adjustSize();

        // m_featIndex->setChecked(on);
        // m_fullTextIndex->setChecked(on);
        if (!on) {
            this->setOcrTextQuery(on);
            this->setFulltextQuery(on);
        }
    } else if (sd == m_ocrIndex) {
        this->setOcrTextQuery(m_ocrIndex->checked());
    } else if (sd == m_fullTextIndex) {
        this->setFulltextQuery(m_fullTextIndex->checked());
    }
}

void IntelligentRetrievalWidget::setOcrTextQuery(bool on)
{
    SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_OCRTEXT, on);
    // 调用文管接口，没开就打开
    if (on) {
        DConfig *dcfg = DConfig::create("org.deepin.dde.file-manager", "org.deepin.dde.file-manager.search");
        bool isTrue = dcfg->value("enableOcrTextSearch").toBool();
        if (!isTrue) {
            qCDebug(logGrandSearch) << "Enabling ocr text search in file manager configuration";
            dcfg->setValue("enableOcrTextSearch", true);
        }
        delete dcfg;
    }
}

void IntelligentRetrievalWidget::setFulltextQuery(bool on)
{
    qCDebug(logGrandSearch) << "Full text search configuration changed - Enabled:" << on;

    SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, on);
    // 调用文管接口，没开就打开
    if (on) {
        DConfig *dcfg = DConfig::create("org.deepin.dde.file-manager", "org.deepin.dde.file-manager.search");
        bool isTrue = dcfg->value("enableFullTextSearch").toBool();
        if (!isTrue) {
            qCDebug(logGrandSearch) << "Enabling full text search in file manager configuration";
            dcfg->setValue("enableFullTextSearch", true);
        }
        delete dcfg;
    }
}
