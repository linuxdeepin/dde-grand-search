// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticsearchwidget.h"
#include "tipslabel.h"
#include "global/searchconfigdefine.h"
#include "business/config/searchconfig.h"

#include <DConfig>

#include <QVBoxLayout>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace GrandSearch;

inline constexpr char kCfgAppId[] { "org.deepin.dde.file-manager" };
inline constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
inline constexpr char kEnableFileIndexSearch[] { "enableFileIndexSearch" };

SemanticSearchWidget::SemanticSearchWidget(QWidget *parent)
    : DWidget(parent)
{
    m_searchDConfig = DConfig::create(kCfgAppId, kSearchCfgPath);
    if (!m_searchDConfig)
        qCWarning(logGrandSearch) << "Failed to create DConfig for file manager search";

    initUi();
    initConnect();
}

SemanticSearchWidget::~SemanticSearchWidget()
{
    delete m_searchDConfig;
}

void SemanticSearchWidget::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 智能搜索开关
    QHBoxLayout *switchLayout = new QHBoxLayout;
    switchLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *groupLabel = new QLabel(tr("Smart search"), this);
    m_switchBtn = new DSwitchButton(this);
    switchLayout->addWidget(groupLabel, 1);
    switchLayout->addWidget(m_switchBtn);

    // 从配置加载初始状态：语义搜索开关需同时满足自身配置和文件索引 DConfig 开启
    bool semanticEnabled = SearchConfig::instance()->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true).toBool();
    bool fileIndexEnabled = isFileIndexSearchEnabled();
    m_switchBtn->setChecked(semanticEnabled && fileIndexEnabled);

    // 描述信息
    TipsLabel *tipsLabel = new TipsLabel(tr("When enabled, natural language search is supported in grand search, e.g., "
                                            "\"PPT documents edited yesterday\".Using this feature requires "
                                            "file indexing to be enabled."), this);
    tipsLabel->setWordWrap(true);

    mainLayout->addLayout(switchLayout);
    mainLayout->addWidget(tipsLabel);
}

void SemanticSearchWidget::initConnect()
{
    connect(m_switchBtn, &DSwitchButton::checkedChanged, this, &SemanticSearchWidget::onSwitchToggled);

    // 监听文管搜索 DConfig 变化，当文件名索引被关闭时同步关闭语义搜索开关
    if (m_searchDConfig) {
        connect(m_searchDConfig, &DConfig::valueChanged, this, &SemanticSearchWidget::onFileIndexChanged);
    }
}

void SemanticSearchWidget::onSwitchToggled(bool checked)
{
    qCDebug(logGrandSearch) << "Smart search configuration changed - Enabled:" << checked;
    SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, checked);

    // 开启智能搜索时需同时开启文件索引；关闭时不操作文件索引配置
    if (checked) {
        setFileIndexSearchEnabled(true);
    }
}

void SemanticSearchWidget::onFileIndexChanged(const QString &key)
{
    if (key != QLatin1String(kEnableFileIndexSearch))
        return;

    // 文件名索引被关闭时，同步关闭语义搜索开关
    if (!isFileIndexSearchEnabled() && m_switchBtn->isChecked()) {
        qCDebug(logGrandSearch) << "File index search disabled, unchecking smart search switch";
        m_switchBtn->setChecked(false);
    }
}

bool SemanticSearchWidget::isFileIndexSearchEnabled() const
{
    if (!m_searchDConfig)
        return false;

    return m_searchDConfig->value(kEnableFileIndexSearch, false).toBool();
}

void SemanticSearchWidget::setFileIndexSearchEnabled(bool enabled)
{
    if (!m_searchDConfig)
        return;

    m_searchDConfig->setValue(kEnableFileIndexSearch, enabled);
}
