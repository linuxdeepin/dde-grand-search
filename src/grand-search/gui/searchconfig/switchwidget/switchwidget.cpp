// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "switchwidget.h"

#include <DPalette>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <DGuiApplicationHelper>
#else
#include <DApplicationHelper>
#endif

#include <QPainter>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

#define ICONLABELSIZE   36

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

SwitchWidget::SwitchWidget(const QString &title, QWidget *parent)
    : SwitchWidget(parent, new QLabel(title, parent))
{
    qCDebug(logGrandSearch) << "Creating SwitchWidget with title:" << title;
}

SwitchWidget::SwitchWidget(QWidget *parent, QWidget *leftWidget)
    : RoundedBackground (parent)
    , m_leftWidget(leftWidget)
{
    qCDebug(logGrandSearch) << "Creating SwitchWidget with custom left widget";

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(ICONLABELSIZE, ICONLABELSIZE);

    if (!m_leftWidget) {
        m_leftWidget = new QLabel(this);
    }
    m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    // 如果是 QLabel，设置自动换行
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        label->setWordWrap(true);
    }

    m_switchBtn = new DSwitchButton(this);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(m_leftWidget);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->addWidget(m_iconLabel);
    m_mainLayout->addLayout(labelLayout, 0);
    m_mainLayout->addWidget(m_switchBtn, 0, Qt::AlignVCenter);
    setLayout(m_mainLayout);

    connect(m_switchBtn, &DSwitchButton::toggled, this, &SwitchWidget::checkedChanged);

    setTopRound(true);
    setBottomRound(true);
    qCDebug(logGrandSearch) << "SwitchWidget created successfully";
}

void SwitchWidget::setChecked(const bool checked)
{
    qCDebug(logGrandSearch) << "Setting switch state - Title:" << title()
                            << "Checked:" << checked;
    m_switchBtn->blockSignals(true);
    m_switchBtn->setChecked(checked);
    m_switchBtn->blockSignals(false);
}

bool SwitchWidget::checked() const
{
    return m_switchBtn->isChecked();
}

void SwitchWidget::setTitle(const QString &title)
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        qCDebug(logGrandSearch) << "Setting switch title - Title:" << title;
        label->setText(title);
        label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        label->adjustSize();
    } else {
        qCWarning(logGrandSearch) << "Failed to set title - Left widget is not a QLabel";
    }
}

QString SwitchWidget::title() const
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        return label->text();
    }

    return QString();
}

void SwitchWidget::setIconEnable(bool e)
{
    qCDebug(logGrandSearch) << "Setting icon visibility - Enabled:" << e;
    m_iconLabel->setVisible(e);
}

void SwitchWidget::setIcon(const QIcon &icon, const QSize &size)
{
    qCDebug(logGrandSearch) << "Setting switch icon - Size:" << size;
    m_iconLabel->setPixmap(icon.pixmap(size));

    update();
}

void SwitchWidget::setEnableBackground(const bool enable)
{
    if (enable == m_hasBack)
        return;
    m_hasBack = enable;
    update();
}

bool SwitchWidget::enableBackground() const
{
    return m_hasBack;
}

void SwitchWidget::paintEvent(QPaintEvent *event)
{
    if (m_hasBack) {
        return RoundedBackground::paintEvent(event);
//        const DPalette &dp = DApplicationHelper::instance()->palette(this);
//        QPainter p(this);
//        p.setPen(Qt::NoPen);
//        p.setBrush(dp.brush(DPalette::ItemBackground));
//        p.drawRoundedRect(rect(), 8, 8);
    }
    return QWidget::paintEvent(event);
}
