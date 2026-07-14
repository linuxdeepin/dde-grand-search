// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "authpromptwidget.h"
#include "gui/iconbutton.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"
#include "utils/utils.h"

#include <dfm-search/dsearch_global.h>

#include <DConfig>
#include <DDciIcon>
#include <DFontSizeManager>
#include <DHorizontalLine>

#include <QFontMetrics>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace GrandSearch;

inline constexpr char kCfgAppId[] { "org.deepin.dde.file-manager" };
inline constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
inline constexpr char kEnableFileIndexSearch[] { "enableFileIndexSearch" };
inline constexpr char kEnableFullTextSearch[] { "enableFullTextSearch" };
inline constexpr char kEnableOcrTextSearch[] { "enableOcrTextSearch" };
inline constexpr char kSearchSettingGroup[] { "10_advance.00_search" };
inline constexpr int kMargin { 20 };

AuthPromptWidget::AuthPromptWidget(QWidget *parent)
    : DWidget(parent)
{
    m_searchDConfig = DConfig::create(kCfgAppId, kSearchCfgPath);
    if (!m_searchDConfig)
        qCWarning(logGrandSearch) << "Failed to create DConfig for file manager search";

    initUi();
    initConnect();
    updatePromptContent();
}

AuthPromptWidget::~AuthPromptWidget()
{
    delete m_searchDConfig;
}

void AuthPromptWidget::updatePromptContent()
{
    // 用户已关闭过，不再显示
    if (SearchConfig::instance()->getConfig(GRANDSEARCH_AUTHPROMPT_GROUP, GRANDSEARCH_AUTHPROMPT_DISMISSED, false).toBool()) {
        qCDebug(logGrandSearch) << "Auth prompt was previously dismissed";
        hide();
        return;
    }

    // 文件索引已开启时无需提示
    if (isFileIndexSearchEnabled()) {
        hide();
        return;
    }

    QStringList disabledModes = disabledSearchModes();
    if (disabledModes.isEmpty()) {
        hide();
        return;
    }

    // 构建功能列表文本
    m_featuresText = QLocale().createSeparatedList(disabledModes);
    show();
}

void AuthPromptWidget::setSearchindexesEnable()
{
    if (!m_searchDConfig)
        return;

    m_searchDConfig->setValue(kEnableFileIndexSearch, true);
    m_searchDConfig->setValue(kEnableFullTextSearch, true);
    m_searchDConfig->setValue(kEnableOcrTextSearch, true);

    SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true);

    const QStringList actions = { "view-index-status", tr("View index status") };
    QJsonObject paramObj;
    paramObj.insert("group", kSearchSettingGroup);
    QJsonObject argsObj;
    argsObj.insert("action", "settings");
    argsObj.insert("params", paramObj);
    const QStringList cmdShowSettings { "file-manager.sh",
                                        "--event",
                                        QJsonDocument(argsObj).toJson(QJsonDocument::Compact) };
    QVariantMap hints = { { "x-deepin-action-view-index-status", cmdShowSettings } };
    Utils::notifyMessage(tr("Smart search"),
                         tr("Indexing is in progress. You can check the index status in file manager settings."),
                         actions, hints);
}

QStringList AuthPromptWidget::disabledSearchModes()
{
    QStringList modes;
    if (!isFullTextSearchEnabled())
        modes << tr("\"Full-Text search\"");
    if (!isOcrTextSearchEnabled())
        modes << tr("\"Image-Content search\"");
    if (!isSemanticSearchEnabled())
        modes << tr("\"Smart search\"");

    return modes;
}

bool AuthPromptWidget::isFileIndexSearchEnabled() const
{
    return m_searchDConfig && m_searchDConfig->value(kEnableFileIndexSearch, false).toBool();
}

bool AuthPromptWidget::isFullTextSearchEnabled() const
{
    return m_searchDConfig && m_searchDConfig->value(kEnableFullTextSearch, false).toBool();
}

bool AuthPromptWidget::isOcrTextSearchEnabled() const
{
    return m_searchDConfig && m_searchDConfig->value(kEnableOcrTextSearch, false).toBool();
}

bool AuthPromptWidget::isSemanticSearchEnabled() const
{
    // 智能搜索需同时满足自身配置开启和文件索引开启
    bool cfgEnabled = SearchConfig::instance()->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true).toBool();
    return cfgEnabled && isFileIndexSearchEnabled();
}

void AuthPromptWidget::initUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(kMargin, 0, kMargin, 0);
    vLayout->setSpacing(8);

    m_hLayout = new QHBoxLayout();
    m_hLayout->setContentsMargins(0, 0, 0, 0);
    m_hLayout->setSpacing(10);

    // 文本标签（单行不换行）
    m_contentLabel = new DTipLabel("", this);
    m_contentLabel->setWordWrap(false);
    m_contentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    QFont contentFont = m_contentLabel->font();
    m_contentLabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8, contentFont));

    // 关闭按钮（hover 时显示）
    m_closeButton = new IconButton(this);
    m_closeButton->setIconSize({ 18, 18 });
    m_closeButton->setIcon(DDciIcon::fromTheme("clear"));
    m_closeButton->setFlat(true);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->hide();

    m_hLayout->addWidget(m_contentLabel, 1);
    m_hLayout->addWidget(m_closeButton, 0);

    // 底部分隔线
    DHorizontalLine *separator = new DHorizontalLine(this);

    vLayout->addLayout(m_hLayout);
    vLayout->addWidget(separator);
}

void AuthPromptWidget::initConnect()
{
    // 一键授权链接点击
    connect(m_contentLabel, &DLabel::linkActivated, this, [this](const QString &link) {
        if (link == QLatin1String("authorize")) {
            setSearchindexesEnable();
            hide();
        }
    });

    // 关闭按钮点击
    connect(m_closeButton, &DIconButton::clicked, this, [this]() {
        qCDebug(logGrandSearch) << "Auth prompt dismissed by user";
        SearchConfig::instance()->setConfig(GRANDSEARCH_AUTHPROMPT_GROUP, GRANDSEARCH_AUTHPROMPT_DISMISSED, true);
        hide();
    });
}

QString AuthPromptWidget::buildElidedText(int availableWidth) const
{
    if (m_featuresText.isEmpty())
        return {};

    // 构建链接（始终完整显示，不省略）
    const auto &linkColor = palette().color(QPalette::Highlight);
    const QString linkText = tr("one-click authorization");
    const QString linkTag = QString("<a href=\"authorize\" style=\"color:%1; text-decoration: none;\">%2</a>")
                                    .arg(linkColor.name(), linkText);

    // 各段固定文本
    const QString prefix = tr("Supports ");
    const QString middle = tr(", click ");
    const QString suffix = tr(" to experience immediately");

    QFontMetrics fm(m_contentLabel->font());

    const int linkW = fm.horizontalAdvance(linkText);
    const int featuresW = fm.horizontalAdvance(m_featuresText);
    const int prefixW = fm.horizontalAdvance(prefix);
    const int middleW = fm.horizontalAdvance(middle);
    const int suffixW = fm.horizontalAdvance(suffix);

    // 空间不足以显示链接
    if (availableWidth <= linkW)
        return linkTag;

    // 尝试完整显示
    if (prefixW + featuresW + middleW + linkW + suffixW <= availableWidth)
        return prefix + m_featuresText + middle + linkTag + suffix;

    // 省略功能列表（中间部分）
    const int neededBase = prefixW + middleW + linkW + suffixW;
    if (neededBase <= availableWidth) {
        QString elided = fm.elidedText(m_featuresText, Qt::ElideMiddle, availableWidth - neededBase);
        return prefix + elided + middle + linkTag + suffix;
    }

    // 省略后缀
    if (prefixW + featuresW + middleW + linkW <= availableWidth)
        return prefix + m_featuresText + middle + linkTag;

    // 极端情况：仅链接
    return linkTag;
}

void AuthPromptWidget::adjustElidedText()
{
    if (m_featuresText.isEmpty()) {
        m_contentLabel->clear();
        return;
    }

    const int closeBtnW = m_closeButton->isVisible()
            ? (m_closeButton->width() + m_hLayout->spacing())
            : 0;
    const int availableWidth = width() - kMargin * 2 - closeBtnW;

    m_contentLabel->setText(buildElidedText(availableWidth));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void AuthPromptWidget::enterEvent(QEnterEvent *event)
#else
void AuthPromptWidget::enterEvent(QEvent *event)
#endif
{
    m_closeButton->show();
    adjustElidedText();
    DWidget::enterEvent(event);
}

void AuthPromptWidget::leaveEvent(QEvent *event)
{
    m_closeButton->hide();
    adjustElidedText();
    DWidget::leaveEvent(event);
}

void AuthPromptWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    adjustElidedText();
}
