/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "detailinfowidget.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QPainterPath>

#define DETAILINFO_ROW_HEIGHT   36
#define KEYLABEL_WIDTH          110
#define VALUELABEL_WIDTH        254
#define HOR_MARGIN_SIZE         10
#define LAYOUT_SPACING          10
#define MARGIN_SIZE             15

DWIDGET_USE_NAMESPACE

SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QWidget(parent)
{
    setMaximumWidth(KEYLABEL_WIDTH);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    m_label = new QLabel(text, this, f);

    hLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));
    hLayout->addWidget(m_label);

    this->setLayout(hLayout);

    QColor textColor = QColor(82, 106, 127, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 1));
    QPalette pa = m_label->palette();
    pa.setColor(QPalette::WindowText, textColor);
    m_label->setPalette(pa);

    m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

void SectionKeyLabel::setText(const QString &text)
{
    m_label->setText(text);
}

QLabel *SectionKeyLabel::label()
{
    return m_label;
}

SectionValueLabel::SectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setMaximumWidth(VALUELABEL_WIDTH);

    setObjectName("SectionValueLabel");

    QColor textColor = QColor(65, 77, 104, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 1));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setWordWrap(true);

    QString t = this->fontMetrics().elidedText(text, Qt::ElideMiddle, VALUELABEL_WIDTH);
    this->setText(t);
    if (t != text)
        this->setToolTip(text);
}


FormLayoutEx::FormLayoutEx(QWidget *parent):
    QFormLayout(parent)
{
    m_vLabels.clear();
    m_vLabels.clear();
}

FormLayoutEx::~FormLayoutEx()
{

}

void FormLayoutEx::addRow(QWidget *label, QWidget *field)
{
    m_vLabels.push_back(label);
    m_vFields.push_back(field);

    QFormLayout::addRow(label, field);
}

QWidget *FormLayoutEx::getRowLabel(int nRow)
{
    if (nRow >= 0 && nRow < m_vLabels.size())
        return m_vLabels[nRow];

    return nullptr;
}

QWidget *FormLayoutEx::getRowField(int nRow)
{
    if (nRow >= 0 && nRow < m_vFields.size())
        return m_vFields[nRow];

    return nullptr;
}

void FormLayoutEx::showRow(int nRow, bool bShow)
{
    QWidget* label = getRowLabel(nRow);
    QWidget* field = getRowField(nRow);
    Q_ASSERT(label);
    Q_ASSERT(field);

    if (bShow) {
        setWidget(nRow, QFormLayout::LabelRole, label);
        setWidget(nRow, QFormLayout::FieldRole, field);
    } else {
        removeWidget(label);
        removeWidget(field);
    }

    label->setVisible(bShow);
    field->setVisible(bShow);
}

void FormLayoutEx::setRowHeight(int nHeight)
{
    for (auto label : m_vLabels) {
        label->setFixedHeight(nHeight);
    }

    for (auto field : m_vFields) {
        field->setFixedHeight(nHeight);
    }
}

int FormLayoutEx::rowCount()
{
    return m_vLabels.size();
}

QRect FormLayoutEx::getRowRect(int nRow)
{
    QRect rt;
    if (nRow < 0 || nRow >= m_vLabels.size())
        return rt;

    QRect rtLabel = m_vLabels.at(nRow)->geometry();
    QRect rtField = m_vFields.at(nRow)->geometry();

    rt = rtLabel.united(rtField);

    return rt;
}

void FormLayoutEx::clear()
{
    qDeleteAll(m_vLabels);
    qDeleteAll(m_vFields);
}

DetailInfoWidget::DetailInfoWidget(QWidget *parent)
    : DWidget(parent)
    , m_nRowHeight(DETAILINFO_ROW_HEIGHT)
{
    m_formLayout = new FormLayoutEx(this);
    m_formLayout->setContentsMargins(HOR_MARGIN_SIZE, MARGIN_SIZE, 0, MARGIN_SIZE);
    m_formLayout->setHorizontalSpacing(LAYOUT_SPACING);
    m_formLayout->setVerticalSpacing(1);

    this->setLayout(m_formLayout);
}

DetailInfoWidget::~DetailInfoWidget()
{

}

void DetailInfoWidget::setDetailInfo(const GrandSearch::DetailInfoList &infos)
{
    m_vInfos = infos;

    if (m_vInfos.isEmpty()) {
        this->hide();
        return;
    }

    int nRowCount = m_formLayout->rowCount();
    int nInfoCount = m_vInfos.size();

    // 表单行不足显示新来的信息，添加新行到表单
    SectionKeyLabel *label = nullptr;
    SectionValueLabel *value = nullptr;
    if (nInfoCount > nRowCount) {
        for (int i = nRowCount; i < nInfoCount; i++) {
            label = new SectionKeyLabel();
            label->setFixedHeight(m_nRowHeight);
            value = new SectionValueLabel();
            value->setFixedHeight(m_nRowHeight);
            value->setWordWrap(false);
            m_formLayout->addRow(label, value);
        }
    }

    nRowCount = m_formLayout->rowCount();
    for (int i = 0; i < nRowCount; i++) {
        m_formLayout->showRow(i, i < nInfoCount);

        if (i < nInfoCount) {
            label = qobject_cast<SectionKeyLabel*>(m_formLayout->getRowLabel(i));
            value = qobject_cast<SectionValueLabel*>(m_formLayout->getRowField(i));
            if (label)
                label->setText(m_vInfos[i].first);
            if (value) {
                QString text = m_vInfos[i].second;
                QString t = this->fontMetrics().elidedText(text, Qt::ElideMiddle, value->width());
                value->setText(t);
                if (t != text)
                    value->setToolTip(text);
            }
            m_formLayout->showRow(i, true);
        }
        else {
            m_formLayout->showRow(i, false);
        }
    }

    this->show();
}

void DetailInfoWidget::setRowHeight(int nHeight)
{
    m_formLayout->setRowHeight(nHeight);

    m_nRowHeight = nHeight;
}

void DetailInfoWidget::clear()
{
    m_formLayout->clear();
}

void DetailInfoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, static_cast<int>(255 * 0.05)));

    int nRadius = 8;
    QRect roundRect(0, 0, nRadius * 2, nRadius * 2);
    int nRowCount = m_formLayout->rowCount();
    if (nRowCount == 1) {
        QRect rtRow = m_formLayout->getRowRect(0);

        QPainterPath path;
        path.moveTo(rtRow.topLeft().x(), rtRow.topLeft().y() + nRadius);
        roundRect.moveTo(rtRow.topLeft());
        path.arcTo(roundRect, 180, -90);
        path.lineTo(rtRow.right() - nRadius, rtRow.top());
        roundRect.moveTo(rtRow.right() - roundRect.width(), rtRow.top());
        path.arcTo(roundRect, 90, -90);

        path.lineTo(rtRow.right(), rtRow.bottom() - nRadius);

        roundRect.moveTo(rtRow.right() - roundRect.width(), rtRow.bottom() - roundRect.height());
        path.arcTo(roundRect, 0, -90);

        path.lineTo(roundRect.right() - nRadius, roundRect.bottom() + 1);

        roundRect.moveTo(rtRow.left(), rtRow.bottom() - roundRect.height());
        path.arcTo(roundRect, 270, -90);

        path.lineTo(rtRow.left(), rtRow.top() + nRadius);

        path.closeSubpath();
        painter.drawPath(path);
    }
    else {
        for (int i = 0; i < nRowCount; i++) {
            QRect rtRow = m_formLayout->getRowRect(i);
            //qDebug() << "DetailInfoWidget::paintEvent i:" << i << "rowRect: " << rtRow;

            // 若只有一个，就绘制一个圆角矩形
            if (i == 0) {
                QPainterPath path;
                path.moveTo(rtRow.topLeft().x(), rtRow.topLeft().y() + nRadius);
                roundRect.moveTo(rtRow.topLeft());
                path.arcTo(roundRect, 180, -90);
                path.lineTo(rtRow.right() - nRadius, rtRow.top());
                roundRect.moveTo(rtRow.right() - roundRect.width(), rtRow.top());
                path.arcTo(roundRect, 90, -90);

                path.lineTo(rtRow.right(), rtRow.bottom() + 1);
                path.lineTo(rtRow.left(), rtRow.bottom() + 1);
                path.lineTo(rtRow.left(), rtRow.top() + nRadius);

                path.closeSubpath();
                painter.drawPath(path);
            }
            else if (i == nRowCount - 1) {
                QPainterPath path;
                path.moveTo(rtRow.topLeft());
                path.lineTo(rtRow.topRight());
                path.lineTo(rtRow.right(), rtRow.bottom() - nRadius);

                roundRect.moveTo(rtRow.right() - roundRect.width(), rtRow.bottom() - roundRect.width());
                path.arcTo(roundRect, 0, -90);

                path.lineTo(roundRect.left() + nRadius, roundRect.bottom() + 1);

                roundRect.moveTo(rtRow.left(), rtRow.bottom() - roundRect.height());
                path.arcTo(roundRect, 270, -90);

                path.lineTo(rtRow.left(), rtRow.top());

                path.closeSubpath();
                painter.drawPath(path);
            }
            else
                painter.drawRect(rtRow.adjusted(0, 0, -1, 0));
        }
    }
    DWidget::paintEvent(event);
}
