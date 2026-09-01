// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHINTWIDGET_H
#define SEARCHHINTWIDGET_H

#include <DWidget>
#include <DTipLabel>

#include <QHBoxLayout>
#include <QSet>

#include "business/dbus/indexstatusmonitor.h"

namespace Dtk { namespace Core { class DConfig; } }

namespace Dtk { namespace Widget { class DSpinner; } }

namespace GrandSearch {

class IconButton;

/**
 * @brief 搜索提示控件
 *
 * 在搜索结果页顶部显示单条最高优先级提示（状态语义对齐文管）：
 * 1. 授权提示（最高）：全文/图片文本/智能搜索不可用时引导一键授权，关闭状态持久化
 * 2. 索引失败：引导重试更新（Retry update）
 * 3. 索引待重建（WaitingUpgrade）：索引库缺失/版本不兼容，引导立即更新
 * 4. 索引暂停（电池供电）：引导继续更新
 * 5. 索引暂停（省电模式）：引导继续更新
 * 6. 等待系统空闲（WaitingIdle）：引导继续更新
 * 7. 索引更新中（带 spinner）：提示当前可用的搜索模式
 *
 * 索引状态经文管 D-Bus 接口（TextIndex/OcrIndex）获取并实时监听。
 * 支持文本省略（保留操作链接完整）、hover 显示关闭按钮；
 * 索引类提示关闭为会话级，索引转入新的更新任务后可再次出现。
 */
class SearchHintWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT

public:
    // 提示类型（对齐文管 SearchHintController/IndexStatusController 状态语义）
    enum class HintType {
        None,                   // 无提示
        AuthHint,               // 授权开启搜索功能（最高优先级）
        IndexFailed,            // 索引更新失败
        IndexWaitingUpgrade,    // 索引待重建（升级/库缺失），结果不完整，影响大于环境暂停
        IndexPausedBattery,     // 电池供电导致索引暂停
        IndexPausedPowerSave,   // 省电模式导致索引暂停
        IndexWaitingIdle,       // 等待系统空闲继续更新（最不紧迫的等待态）
        IndexUpdating,          // 索引更新中
    };

    explicit SearchHintWidget(QWidget *parent = nullptr);
    ~SearchHintWidget() override;

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initUi();
    void initConnect();

    // 提示评估与刷新（优先级：授权 > 失败 > 电池暂停 > 省电暂停 > 更新中）
    void evaluateAndShow();
    HintType evaluateHint() const;

    // 检查文管搜索 DConfig 中各功能项的开启状态
    bool isFileIndexSearchEnabled() const;
    bool isFullTextSearchEnabled() const;
    bool isOcrTextSearchEnabled() const;
    bool isSemanticSearchEnabled() const;

    // 返回当前未开启的搜索模式名称列表（为空表示全部已开启）
    QStringList disabledSearchModes() const;

    // 授权提示
    bool shouldShowAuthHint() const;
    void setSearchindexesEnable();

    // 索引状态监听（进程生命周期内持续监听，仅状态失效时重新查询）
    void startListening();
    void updateIndexStatus(IndexStatusMonitor::Index idx, const QString &state, const QString &grade);

    // 操作处理
    void onLinkActivated(const QString &link);
    void dismissAndReevaluate();
    // 按提示类型对处于对应状态的索引发起更新请求：
    // Failed/WaitingUpgrade → manual=true（强制），WaitingPower/PowerSave/Idle → manual=false（绕过限制）
    void requestUpdateForHint(HintType type) const;
    void openSearchSettingPage() const;

    // 文案与文本省略计算
    QString hintText(HintType type) const;
    QString updatingHintText() const;
    QString buildElidedText(int availableWidth) const;
    QString buildAuthElidedText(int availableWidth) const;
    QString buildIndexElidedText(int availableWidth) const;
    void adjustElidedText();

private:
    QHBoxLayout *m_hLayout = nullptr;
    Dtk::Widget::DSpinner *m_spinner = nullptr;
    Dtk::Widget::DTipLabel *m_contentLabel = nullptr;
    IconButton *m_closeButton = nullptr;
    Dtk::Core::DConfig *m_searchDConfig = nullptr;
    IndexStatusMonitor *m_indexMonitor = nullptr;

    HintType m_currentType = HintType::None;
    QString m_currentText;                  // 当前提示描述文本
    QString m_featuresText;                 // 授权提示的不可用功能列表文本
    QSet<HintType> m_dismissedTypes;        // 会话内已关闭的索引提示类型

    // 文管索引状态缓存
    QString m_textState;
    QString m_textGrade;
    QString m_ocrState;
    QString m_ocrGrade;
    bool m_textStatusValid = false;
    bool m_ocrStatusValid = false;
    bool m_listening = false;
};

inline uint qHash(SearchHintWidget::HintType type, uint seed = 0) noexcept
{
    return ::qHash(static_cast<int>(type), seed);
}

}

#endif   // SEARCHHINTWIDGET_H
