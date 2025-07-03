// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "comboboxwidget.h"
#include "global/searchhelper.h"

#include <DPalette>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <DGuiApplicationHelper>
#else
#include <DApplicationHelper>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <DPaletteHelper>
#endif

#include <QPainter>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define COMBOBOXW 250
#define COMBOBOXH 36

ComboboxWidget::ComboboxWidget(const QString &title, QWidget *parent)
    : ComboboxWidget(parent, new QLabel(title, parent))
{
    qCDebug(logGrandSearch) << "Creating ComboboxWidget with title:" << title;
}

ComboboxWidget::ComboboxWidget(QWidget *parent, QWidget *leftWidget)
    : QWidget (parent)
    , m_leftWidget(leftWidget)
{
    qCDebug(logGrandSearch) << "Creating ComboboxWidget with custom left widget";
    if (!m_leftWidget)
        m_leftWidget = new QLabel(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_comboBox = new DComboBox(this);
    m_comboBox->setFixedSize(COMBOBOXW, COMBOBOXH);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(m_leftWidget);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 6, 10, 6);
    m_mainLayout->addLayout(labelLayout, 0);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_comboBox, 0, Qt::AlignVCenter);
    setLayout(m_mainLayout);

    connect(m_comboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, &ComboboxWidget::checkedChanged);
    qCDebug(logGrandSearch) << "ComboboxWidget created successfully";
}

void ComboboxWidget::setChecked(const QString &text)
{
    qCDebug(logGrandSearch) << "Setting combobox selection - Text:" << text;
    m_comboBox->setCurrentText(text);
}

QString ComboboxWidget::checked() const
{
    return m_comboBox->currentText();
}

void ComboboxWidget::setTitle(const QString &title)
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        qCDebug(logGrandSearch) << "Setting combobox title - Title:" << title;
        label->setText(title);
        label->adjustSize();
    } else {
        qCWarning(logGrandSearch) << "Failed to set title - Left widget is not a QLabel";
    }
}

QString ComboboxWidget::title() const
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        return label->text();
    }

    return QString();
}

void ComboboxWidget::setEnableBackground(const bool enable)
{
    if (enable == m_hasBack)
        return;
    m_hasBack = enable;
    update();
}

bool ComboboxWidget::enableBackground() const
{
    return m_hasBack;
}

DComboBox *ComboboxWidget::getComboBox()
{
    return m_comboBox;
}

void ComboboxWidget::paintEvent(QPaintEvent *event)
{
    if (m_hasBack) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const DPalette &dp = DPaletteHelper::instance()->palette(this);
#else
        const DPalette &dp = DApplicationHelper::instance()->palette(this);
#endif
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(dp.brush(DPalette::ItemBackground));
        p.drawRoundedRect(rect(), 8, 8);
    }
    return QWidget::paintEvent(event);
}
