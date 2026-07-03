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
#include <QScopedPointer>

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
    initUi();
    initConnect();
    updatePromptContent();
}

AuthPromptWidget::~AuthPromptWidget()
{
}

void AuthPromptWidget::updatePromptContent()
{
    // 用户已关闭过，不再显示
    if (SearchConfig::instance()->getConfig(GRANDSEARCH_AUTHPROMPT_GROUP, GRANDSEARCH_AUTHPROMPT_DISMISSED, false).toBool()) {
        qCDebug(logGrandSearch) << "Auth prompt was previously dismissed";
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

// 创建文件管理器搜索 DConfig，失败时返回 nullptr 并记录日志
static QScopedPointer<DConfig> createSearchDConfig()
{
    DConfig *raw = DConfig::create(kCfgAppId, kSearchCfgPath);
    if (!raw)
        qCWarning(logGrandSearch) << "Failed to create DConfig for file manager search";
    return QScopedPointer<DConfig>(raw);
}

void AuthPromptWidget::enableSearchindexes()
{
    auto dcfg = createSearchDConfig();
    if (!dcfg)
        return;

    // 非智能搜索未开启时，需要进行通知
    bool needNotified = !dcfg->value(kEnableFullTextSearch, false).toBool()
            && !dcfg->value(kEnableOcrTextSearch, false).toBool();

    dcfg->setValue(kEnableFileIndexSearch, true);
    dcfg->setValue(kEnableFullTextSearch, true);
    dcfg->setValue(kEnableOcrTextSearch, true);

    SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true);

    if (needNotified) {
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
}

QStringList AuthPromptWidget::disabledSearchModes()
{
    auto dcfg = createSearchDConfig();
    if (!dcfg)
        return {};

    QStringList modes;
    if (!dcfg->value(kEnableFullTextSearch, false).toBool())
        modes << tr("\"Full-Text search\"");
    if (!dcfg->value(kEnableOcrTextSearch, false).toBool())
        modes << tr("\"Image-Content search\"");
    bool cfgEnabled = SearchConfig::instance()->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true).toBool();
    if (!cfgEnabled || !dcfg->value(kEnableFileIndexSearch, false).toBool())
        modes << tr("\"Smart search\"");

    return modes;
}

void AuthPromptWidget::initUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(kMargin, 0, kMargin, 0);
    vLayout->setSpacing(10);

    m_hLayout = new QHBoxLayout();
    m_hLayout->setContentsMargins(0, 0, 0, 0);
    m_hLayout->setSpacing(10);

    // 文本标签（单行不换行）
    m_contentLabel = new DLabel(this);
    m_contentLabel->setWordWrap(false);
    m_contentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    QFont contentFont = m_contentLabel->font();
    m_contentLabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T6, contentFont));

    // 关闭按钮（hover 时显示）
    m_closeButton = new IconButton(this);
    m_closeButton->setIconSize({ 20, 20 });
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
            enableSearchindexes();
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
