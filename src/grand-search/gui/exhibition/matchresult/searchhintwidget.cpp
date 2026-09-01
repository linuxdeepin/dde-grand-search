// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhintwidget.h"
#include "gui/iconbutton.h"
#include "business/config/searchconfig.h"
#include "business/dbus/indexstatusmonitor.h"
#include "global/searchconfigdefine.h"
#include "utils/utils.h"

#include <dfm-search/dsearch_global.h>

#include <DConfig>
#include <DDciIcon>
#include <DFontSizeManager>
#include <DHorizontalLine>
#include <DSpinner>

#include <QFontMetrics>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QLocale>
#include <QProcess>
#include <QTimer>

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

// 文管索引状态值（见文管 TaskManager::currentIndexStatus）
inline constexpr QLatin1String kStateIdle { "Idle" };
inline constexpr QLatin1String kStateRunning { "Running" };
inline constexpr QLatin1String kStateFailed { "Failed" };
inline constexpr QLatin1String kStateWaitingPower { "WaitingPower" };
inline constexpr QLatin1String kStateWaitingPowerSave { "WaitingPowerSave" };
inline constexpr QLatin1String kStateWaitingIdle { "WaitingIdle" };
inline constexpr QLatin1String kStateWaitingUpgrade { "WaitingUpgrade" };

// 跳转文管搜索设置页的命令
static QStringList searchSettingCommand()
{
    QJsonObject paramObj;
    paramObj.insert("group", kSearchSettingGroup);
    QJsonObject argsObj;
    argsObj.insert("action", "settings");
    argsObj.insert("params", paramObj);
    return { "file-manager.sh",
             "--event",
             QJsonDocument(argsObj).toJson(QJsonDocument::Compact) };
}

SearchHintWidget::SearchHintWidget(QWidget *parent)
    : DWidget(parent)
{
    m_searchDConfig = DConfig::create(kCfgAppId, kSearchCfgPath);
    if (!m_searchDConfig)
        qCWarning(logGrandSearch) << "Failed to create DConfig for file manager search";

    m_indexMonitor = new IndexStatusMonitor(this);

    initUi();
    initConnect();
    evaluateAndShow();
    // 延迟到事件循环再启动 D-Bus 监听，避免服务冷启动的同步拉起阻塞窗口首显；
    // 控件可能因“无提示”保持隐藏（不会触发 showEvent），故不能只依赖 showEvent 启动
    QTimer::singleShot(0, this, [this]() { startListening(); });
}

SearchHintWidget::~SearchHintWidget()
{
    delete m_searchDConfig;
}

bool SearchHintWidget::shouldShowAuthHint() const
{
    // 用户已关闭过授权提示，不再显示
    if (SearchConfig::instance()->getConfig(GRANDSEARCH_AUTHPROMPT_GROUP, GRANDSEARCH_AUTHPROMPT_DISMISSED, false).toBool())
        return false;

    // 文件索引已开启时无需提示
    if (isFileIndexSearchEnabled())
        return false;

    return !disabledSearchModes().isEmpty();
}

void SearchHintWidget::setSearchindexesEnable()
{
    if (!m_searchDConfig)
        return;

    m_searchDConfig->setValue(kEnableFileIndexSearch, true);
    m_searchDConfig->setValue(kEnableFullTextSearch, true);
    m_searchDConfig->setValue(kEnableOcrTextSearch, true);

    SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true);

    const QStringList actions = { "view-index-status", tr("View index status") };
    const QStringList cmdShowSettings = searchSettingCommand();
    QVariantMap hints = { { "x-deepin-action-view-index-status", cmdShowSettings } };
    Utils::notifyMessage(tr("Smart search"),
                         tr("Indexing is in progress. You can check the index status in file manager settings."),
                         actions, hints);
}

void SearchHintWidget::openSearchSettingPage() const
{
    const QStringList cmdShowSettings = searchSettingCommand();
    QProcess::startDetached(cmdShowSettings.value(0), cmdShowSettings.mid(1));
}

QStringList SearchHintWidget::disabledSearchModes() const
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

bool SearchHintWidget::isFileIndexSearchEnabled() const
{
    return m_searchDConfig && m_searchDConfig->value(kEnableFileIndexSearch, false).toBool();
}

bool SearchHintWidget::isFullTextSearchEnabled() const
{
    return m_searchDConfig && m_searchDConfig->value(kEnableFullTextSearch, false).toBool();
}

bool SearchHintWidget::isOcrTextSearchEnabled() const
{
    return m_searchDConfig && m_searchDConfig->value(kEnableOcrTextSearch, false).toBool();
}

bool SearchHintWidget::isSemanticSearchEnabled() const
{
    // 智能搜索需同时满足自身配置开启和文件索引开启
    bool cfgEnabled = SearchConfig::instance()->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true).toBool();
    return cfgEnabled && isFileIndexSearchEnabled();
}

void SearchHintWidget::startListening()
{
    if (m_listening) {
        // 已在监听：状态失效（如服务此前不可用）时重新查询
        if (!m_textStatusValid || !m_ocrStatusValid)
            m_indexMonitor->getIndexStatus();
        return;
    }

    qCDebug(logGrandSearch) << "SearchHintWidget: start listening to index status";
    m_listening = true;

    m_textStatusValid = false;
    m_ocrStatusValid = false;
    m_indexMonitor->getIndexStatus();
}

void SearchHintWidget::evaluateAndShow()
{
    const HintType newType = evaluateHint();
    if (newType == HintType::None) {
        m_currentType = HintType::None;
        m_currentText.clear();
        m_featuresText.clear();
        m_spinner->stop();
        m_spinner->hide();
        m_contentLabel->clear();
        hide();
        return;
    }

    if (newType == HintType::AuthHint)
        m_featuresText = QLocale().createSeparatedList(disabledSearchModes());

    const QString newText = hintText(newType);
    if (newType == m_currentType && newText == m_currentText)
        return;

    const HintType oldType = m_currentType;
    m_currentType = newType;
    m_currentText = newText;

    if (newType == HintType::IndexUpdating) {
        m_spinner->show();
        m_spinner->start();
    } else if (oldType == HintType::IndexUpdating) {
        m_spinner->stop();
        m_spinner->hide();
    }

    adjustElidedText();
    show();
}

SearchHintWidget::HintType SearchHintWidget::evaluateHint() const
{
    // 1. 授权提示（最高优先级）
    if (!m_dismissedTypes.contains(HintType::AuthHint) && shouldShowAuthHint())
        return HintType::AuthHint;

    // 索引提示仅在文件索引搜索开启时显示（与文管一致）
    if (!isFileIndexSearchEnabled())
        return HintType::None;

    // 2. 索引失败
    if (!m_dismissedTypes.contains(HintType::IndexFailed)) {
        if ((m_textStatusValid && m_textState == kStateFailed)
            || (m_ocrStatusValid && m_ocrState == kStateFailed))
            return HintType::IndexFailed;
    }

    // 3. 索引待重建（库缺失/版本不兼容，结果不完整，影响大于环境暂停）
    if (!m_dismissedTypes.contains(HintType::IndexWaitingUpgrade)) {
        if ((m_textStatusValid && m_textState == kStateWaitingUpgrade)
            || (m_ocrStatusValid && m_ocrState == kStateWaitingUpgrade))
            return HintType::IndexWaitingUpgrade;
    }

    // 4. 电池供电暂停
    if (!m_dismissedTypes.contains(HintType::IndexPausedBattery)) {
        if ((m_textStatusValid && m_textState == kStateWaitingPower)
            || (m_ocrStatusValid && m_ocrState == kStateWaitingPower))
            return HintType::IndexPausedBattery;
    }

    // 5. 省电模式暂停
    if (!m_dismissedTypes.contains(HintType::IndexPausedPowerSave)) {
        if ((m_textStatusValid && m_textState == kStateWaitingPowerSave)
            || (m_ocrStatusValid && m_ocrState == kStateWaitingPowerSave))
            return HintType::IndexPausedPowerSave;
    }

    // 6. 等待系统空闲（最不紧迫的等待态，可自动恢复）
    if (!m_dismissedTypes.contains(HintType::IndexWaitingIdle)) {
        if ((m_textStatusValid && m_textState == kStateWaitingIdle)
            || (m_ocrStatusValid && m_ocrState == kStateWaitingIdle))
            return HintType::IndexWaitingIdle;
    }

    // 7. 更新中
    if (!m_dismissedTypes.contains(HintType::IndexUpdating)) {
        if ((m_textStatusValid && m_textState == kStateRunning)
            || (m_ocrStatusValid && m_ocrState == kStateRunning))
            return HintType::IndexUpdating;
    }

    return HintType::None;
}

QString SearchHintWidget::hintText(HintType type) const
{
    switch (type) {
    case HintType::AuthHint:
        return m_featuresText;
    case HintType::IndexFailed:
        return tr("Some index updates failed. Search results may be incomplete.");
    case HintType::IndexWaitingUpgrade:
        return tr("Waiting for index service upgrade.");
    case HintType::IndexPausedBattery:
        return tr("Running on battery. Some content indexing has been paused.");
    case HintType::IndexPausedPowerSave:
        return tr("Power save mode is enabled. Some content indexing has been paused.");
    case HintType::IndexWaitingIdle:
        return tr("Waiting for the device to become idle to continue updating.");
    case HintType::IndexUpdating:
        return updatingHintText();
    default:
        return QString();
    }
}

QString SearchHintWidget::updatingHintText() const
{
    // 按就绪情况提示当前可用的搜索模式（对齐文管 updatingHintText）
    const bool textReady = m_textStatusValid && m_textState == kStateIdle;
    const bool ocrReady = m_ocrStatusValid && m_ocrState == kStateIdle;

    if (textReady && !ocrReady)
        return tr("Index is being updated. File name and file content search are available.");
    if (!textReady && ocrReady)
        return tr("Index is being updated. File name and image content search are available.");
    return tr("Index is being updated. File name search is available.");
}

void SearchHintWidget::onLinkActivated(const QString &link)
{
    if (link == QLatin1String("authorize")) {
        setSearchindexesEnable();
        hide();
    } else if (link == QLatin1String("retry-update")
               || link == QLatin1String("continue-update")) {
        // 更新类操作统一按当前提示类型分发（目标状态与 manual 语义见 requestUpdateForHint）
        requestUpdateForHint(m_currentType);
        dismissAndReevaluate();
    } else if (link == QLatin1String("view-status")) {
        openSearchSettingPage();
        dismissAndReevaluate();
    }
}

void SearchHintWidget::dismissAndReevaluate()
{
    const HintType actedType = m_currentType;
    m_dismissedTypes.insert(actedType);
    m_currentType = HintType::None;
    m_currentText.clear();

    // 延迟到事件循环，避免在链接信号内重入刷新（对齐文管 dismissAndReevaluate）
    QTimer::singleShot(0, this, [this]() { evaluateAndShow(); });
}

void SearchHintWidget::requestUpdateForHint(HintType type) const
{
    // 提示类型 → 目标状态 + manual 语义：
    // Failed/WaitingUpgrade → manual=true（强制重试/立即重建，对齐文管设置页 href="manual"）
    // WaitingPower/WaitingPowerSave/WaitingIdle → manual=false（绕过环境限制，href="bypass"）
    QLatin1String targetState("");
    bool manual = false;
    switch (type) {
    case HintType::IndexFailed:
        targetState = kStateFailed;
        manual = true;
        break;
    case HintType::IndexWaitingUpgrade:
        targetState = kStateWaitingUpgrade;
        manual = true;
        break;
    case HintType::IndexPausedBattery:
        targetState = kStateWaitingPower;
        manual = false;
        break;
    case HintType::IndexPausedPowerSave:
        targetState = kStateWaitingPowerSave;
        manual = false;
        break;
    case HintType::IndexWaitingIdle:
        targetState = kStateWaitingIdle;
        manual = false;
        break;
    default:
        return;   // 其余提示类型无更新操作
    }

    // 仅对确实处于目标状态的索引发起请求
    const QStringList paths = DFMSEARCH::Global::defaultIndexedDirectory();
    if (m_textStatusValid && m_textState == targetState)
        m_indexMonitor->forceUpdateIndex(IndexStatusMonitor::Index::Text, paths, manual);
    if (m_ocrStatusValid && m_ocrState == targetState)
        m_indexMonitor->forceUpdateIndex(IndexStatusMonitor::Index::Ocr, paths, manual);
}

void SearchHintWidget::initUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(kMargin, 0, kMargin, 0);
    vLayout->setSpacing(8);

    m_hLayout = new QHBoxLayout();
    m_hLayout->setContentsMargins(0, 0, 0, 0);
    m_hLayout->setSpacing(10);

    // 更新中提示的加载动画
    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(16, 16);
    m_spinner->hide();

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

    m_hLayout->addWidget(m_spinner, 0, Qt::AlignVCenter);
    m_hLayout->addWidget(m_contentLabel, 1);
    m_hLayout->addWidget(m_closeButton, 0);

    // 底部分隔线
    DHorizontalLine *separator = new DHorizontalLine(this);

    vLayout->addLayout(m_hLayout);
    vLayout->addWidget(separator);
}

void SearchHintWidget::initConnect()
{
    // 一键授权 / 索引操作链接点击
    connect(m_contentLabel, &DLabel::linkActivated, this, [this](const QString &link) {
        onLinkActivated(link);
    });

    // 关闭按钮点击
    connect(m_closeButton, &DIconButton::clicked, this, [this]() {
        qCDebug(logGrandSearch) << "Search hint dismissed by user, type:" << static_cast<int>(m_currentType);
        if (m_currentType == HintType::AuthHint)
            SearchConfig::instance()->setConfig(GRANDSEARCH_AUTHPROMPT_GROUP, GRANDSEARCH_AUTHPROMPT_DISMISSED, true);
        dismissAndReevaluate();
    });

    // 索引状态查询结果（success=false 表示服务不可用，保持状态无效即不显示索引提示）
    connect(m_indexMonitor, &IndexStatusMonitor::indexStatusResult,
            this, [this](IndexStatusMonitor::Index idx, const QString &state, const QString &grade, bool success) {
                if (!success)
                    return;
                updateIndexStatus(idx, state, grade);
            });

    // 索引状态实时变化
    connect(m_indexMonitor, &IndexStatusMonitor::indexStatusChanged,
            this, [this](IndexStatusMonitor::Index idx, const QString &state, const QString &grade) {
                updateIndexStatus(idx, state, grade);
            });

    // 文管搜索配置变化（如在文管设置中开/关文件索引）时重新评估
    if (m_searchDConfig) {
        connect(m_searchDConfig, &DConfig::valueChanged, this, [this](const QString &key) {
            qCDebug(logGrandSearch) << "File manager search config changed:" << key;
            evaluateAndShow();
        });
    }
}

void SearchHintWidget::updateIndexStatus(IndexStatusMonitor::Index idx, const QString &state, const QString &grade)
{
    bool &valid = (idx == IndexStatusMonitor::Index::Text) ? m_textStatusValid : m_ocrStatusValid;
    QString &curState = (idx == IndexStatusMonitor::Index::Text) ? m_textState : m_ocrState;
    QString &curGrade = (idx == IndexStatusMonitor::Index::Text) ? m_textGrade : m_ocrGrade;

    // 索引转入新的更新任务时，恢复此前被用户关闭的索引提示（对齐文管）
    if (valid && curState != state && curState != kStateRunning && state == kStateRunning)
        m_dismissedTypes.clear();

    curState = state;
    curGrade = grade;
    valid = true;

    evaluateAndShow();
}

QString SearchHintWidget::buildElidedText(int availableWidth) const
{
    if (m_currentType == HintType::AuthHint)
        return buildAuthElidedText(availableWidth);
    return buildIndexElidedText(availableWidth);
}

QString SearchHintWidget::buildAuthElidedText(int availableWidth) const
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

QString SearchHintWidget::buildIndexElidedText(int availableWidth) const
{
    const QString desc = m_currentText.toHtmlEscaped();
    if (desc.isEmpty())
        return {};

    // 各提示类型的操作链接（始终完整显示，不省略）
    struct Link
    {
        QString href;
        QString text;
    };
    QList<Link> links;
    if (m_currentType == HintType::IndexFailed) {
        links << Link { QStringLiteral("retry-update"), tr("Retry update") };
        links << Link { QStringLiteral("view-status"), tr("View") };
    } else if (m_currentType == HintType::IndexWaitingUpgrade) {
        links << Link { QStringLiteral("retry-update"), tr("Retry update") };
        links << Link { QStringLiteral("view-status"), tr("View") };
    } else if (m_currentType == HintType::IndexPausedBattery
               || m_currentType == HintType::IndexPausedPowerSave
               || m_currentType == HintType::IndexWaitingIdle) {
        links << Link { QStringLiteral("continue-update"), tr("Continue updating") };
        links << Link { QStringLiteral("view-status"), tr("View") };
    } else if (m_currentType == HintType::IndexUpdating) {
        links << Link { QStringLiteral("view-status"), tr("View") };
    }

    const auto &linkColor = palette().color(QPalette::Highlight).name();
    const QString separator = QStringLiteral(" | ");
    const QString gap = QStringLiteral(" ");

    QFontMetrics fm(m_contentLabel->font());

    // 构建链接区并计算其宽度
    int linksW = 0;
    QString linksHtml;
    for (int i = 0; i < links.size(); ++i) {
        if (i > 0) {
            linksHtml += separator;
            linksW += fm.horizontalAdvance(separator);
        }
        linksHtml += QString("<a href=\"%1\" style=\"color:%2; text-decoration: none;\">%3</a>")
                             .arg(links.at(i).href, linkColor, links.at(i).text.toHtmlEscaped());
        linksW += fm.horizontalAdvance(links.at(i).text);
    }

    const int descW = fm.horizontalAdvance(desc);
    const int gapW = fm.horizontalAdvance(gap);

    // 空间不足以显示链接
    if (availableWidth <= linksW)
        return linksHtml;

    // 尝试完整显示
    if (descW + gapW + linksW <= availableWidth)
        return desc + gap + linksHtml;

    // 省略描述文本（保留链接完整）
    const QString elided = fm.elidedText(desc, Qt::ElideRight, availableWidth - gapW - linksW);
    return elided + gap + linksHtml;
}

void SearchHintWidget::adjustElidedText()
{
    if (m_currentType == HintType::None) {
        m_contentLabel->clear();
        return;
    }

    const int closeBtnW = m_closeButton->isVisible()
            ? (m_closeButton->width() + m_hLayout->spacing())
            : 0;
    const int spinnerW = m_spinner->isVisibleTo(this)
            ? (m_spinner->width() + m_hLayout->spacing())
            : 0;
    const int availableWidth = width() - kMargin * 2 - closeBtnW - spinnerW;

    m_contentLabel->setText(buildElidedText(availableWidth));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SearchHintWidget::enterEvent(QEnterEvent *event)
#else
void SearchHintWidget::enterEvent(QEvent *event)
#endif
{
    m_closeButton->show();
    adjustElidedText();
    DWidget::enterEvent(event);
}

void SearchHintWidget::leaveEvent(QEvent *event)
{
    m_closeButton->hide();
    adjustElidedText();
    DWidget::leaveEvent(event);
}

void SearchHintWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    adjustElidedText();
}

void SearchHintWidget::showEvent(QShowEvent *event)
{
    DWidget::showEvent(event);
    startListening();
}
