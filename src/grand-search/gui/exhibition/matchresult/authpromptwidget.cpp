// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "authpromptwidget.h"
#include "gui/iconbutton.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"

#include <dfm-search/dsearch_global.h>

#include <DDciIcon>
#include <DFontSizeManager>

#include <QFontMetrics>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

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

QStringList AuthPromptWidget::checkUnavailableFeatures() const
{
    QStringList features;

    // 全文搜索索引
    if (!DFMSEARCH::Global::isContentIndexAvailable())
        features << tr("full-text search");

    // 图片文本内容搜索（OCR）索引
    if (!DFMSEARCH::Global::isOcrTextIndexAvailable())
        features << tr("image text content search");

    // 智能搜索：需要用户启用 + 文件名索引可用
    const bool enabled = SearchConfig::instance()->getConfig(GRANDSEARCH_SMARTSEARCH_GROUP, GRANDSEARCH_SMARTSEARCH_ENABLED, false).toBool();
    const bool indexReady = DFMSEARCH::Global::isFileNameIndexReadyForSearch();
    if (!enabled || !indexReady)
        features << tr("smart search");

    qCDebug(logGrandSearch) << "Unavailable features:" << features
                            << "(smartSearch enabled:" << enabled
                            << ", fileNameIndex ready:" << indexReady << ")";

    return features;
}

void AuthPromptWidget::updatePromptContent()
{
    // 用户已关闭过，不再显示
    if (SearchConfig::instance()->getConfig(GRANDSEARCH_AUTHPROMPT_GROUP, GRANDSEARCH_AUTHPROMPT_DISMISSED, false).toBool()) {
        qCDebug(logGrandSearch) << "Auth prompt was previously dismissed";
        hide();
        return;
    }

    // 检查索引可用性
    QStringList unavailable = checkUnavailableFeatures();
    if (unavailable.isEmpty()) {
        hide();
        return;
    }

    // 构建功能列表文本（中文引号包裹，逗号分隔）
    QStringList quoted;
    for (const QString &f : unavailable)
        quoted << QString("\"%1\"").arg(f);
    m_featuresText = quoted.join(tr(", "));

    show();
    adjustElidedText();
}

void AuthPromptWidget::initUi()
{
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setContentsMargins(10, 0, 10, 0);
    m_vLayout->setSpacing(10);

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
    m_separator = new DHorizontalLine(this);

    m_vLayout->addLayout(m_hLayout);
    m_vLayout->addWidget(m_separator);
}

void AuthPromptWidget::initConnect()
{
    // 一键授权链接点击
    connect(m_contentLabel, &DLabel::linkActivated, this, [this](const QString &link) {
        if (link == QLatin1String("authorize")) {
            // TODO: 实现授权逻辑
            SearchConfig::instance()->setConfig(GRANDSEARCH_SMARTSEARCH_GROUP, GRANDSEARCH_SMARTSEARCH_ENABLED, true);
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
    const QString linkText = tr("Authorize with one click");
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
    const int availableWidth = width() - 10 * 2 - closeBtnW;

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
