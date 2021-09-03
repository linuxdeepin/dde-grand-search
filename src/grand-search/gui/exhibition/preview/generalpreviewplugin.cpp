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
#include "generalpreviewplugin_p.h"
#include "generalpreviewplugin.h"
#include "utils/utils.h"

#include <DFontSizeManager>
#include <DMessageBox>
#include <DGuiApplicationHelper>

#include <QIcon>
#include <QGuiApplication>
#include <QClipboard>

#define ICON_SIZE               80
#define HOR_MARGIN_SIZE         10
#define LAYOUT_SPACING          15
#define MARGIN_SIZE             15

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

#define GET_BASICINFO_VALUE_LABEL(ROW, LABEL) SectionValueLabel* LABEL = qobject_cast<SectionValueLabel*>(m_basicInfoLayout->getRowField(ROW)); \
                                           Q_ASSERT(LABEL);

#define SET_BASICINFO_VALUE_LABEL_TEXT(ROW, TEXT) { \
                                                    GET_BASICINFO_VALUE_LABEL(ROW, LABEL) \
                                                    QString t = LABEL->fontMetrics().elidedText(TEXT, Qt::ElideRight, LABEL->width());\
                                                    LABEL->setText(t); \
                                                    if (t != TEXT) \
                                                        LABEL->setToolTip(TEXT); \
                                                  }

NameLabel::NameLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("NameLabel");
    QColor textColor = QColor(0, 0, 0, static_cast<int>(255 * 0.4));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.4));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    this->setSizePolicy(sizePolicy);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionKeyLabel");
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6, this->font());
    setFont(font);

    QColor textColor = QColor(0, 0, 0, static_cast<int>(255 * 0.6));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.6));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}


SectionValueLabel::SectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionValueLabel");
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6, this->font());
    setFont(font);

    QColor textColor = QColor(0, 0, 0, static_cast<int>(255 * 0.4));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.4));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setWordWrap(true);
}

ActionLabel::ActionLabel(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel (text, parent, f)
{
    setObjectName("ActionLabel");
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6, this->font());
    setFont(font);

    m_textColor = QColor(0, 0, 255, static_cast<int>(255*0.6));
    m_textColor_clicked = QColor(0, 0, 255, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        m_textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.6));
        m_textColor_clicked = QColor(255, 255, 255, static_cast<int>(255 * 1));
    }

    QPalette labelPalette = palette();
    labelPalette.setColor(QPalette::WindowText, m_textColor);
    setPalette(labelPalette);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);
}

void ActionLabel::mousePressEvent(QMouseEvent *event)
{
    QPalette labelPalette = palette();
    labelPalette.setColor(QPalette::WindowText, m_textColor_clicked);
    setPalette(labelPalette);

    QLabel::mousePressEvent(event);
}

void ActionLabel::mouseReleaseEvent(QMouseEvent *event)
{
    QPalette labelPalette = palette();
    labelPalette.setColor(QPalette::WindowText, m_textColor);
    setPalette(labelPalette);

    QLabel::mouseReleaseEvent(event);

    emit sigLabelClicked();
}

void ActionLabel::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);

    QFont font = this->font();
    font.setUnderline(true);
    this->setFont(font);
    repaint();

    QLabel::enterEvent(event);
}

void ActionLabel::leaveEvent(QEvent *event)
{
    setCursor(Qt::ArrowCursor);

    QFont font = this->font();
    font.setUnderline(false);
    this->setFont(font);
    repaint();

    QLabel::leaveEvent(event);
}

VBoxLayoutEx::VBoxLayoutEx(QWidget *parent):
    QVBoxLayout(parent)
{

}

VBoxLayoutEx::~VBoxLayoutEx()
{

}

void VBoxLayoutEx::addRow(QWidget *label, QWidget *field)
{
    m_vLabels.push_back(label);
    m_vFields.push_back(field);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(label);
    hLayout->addWidget(field);
    hLayout->setStretch(0, 1);
    hLayout->setStretch(1, 10);
    m_vHLayout.push_back(hLayout);

    this->addLayout(hLayout);
}

QWidget *VBoxLayoutEx::getRowLabel(int nRow)
{
    if (nRow >= 0 && nRow < m_vLabels.size())
        return m_vLabels[nRow];

    return nullptr;
}

QWidget *VBoxLayoutEx::getRowField(int nRow)
{
    if (nRow >= 0 && nRow < m_vFields.size())
        return m_vFields[nRow];

    return nullptr;
}

QHBoxLayout *VBoxLayoutEx::getRowLayout(int nRow)
{
    if (nRow >= 0 && nRow < m_vHLayout.size())
        return m_vHLayout[nRow];

    return nullptr;
}

void VBoxLayoutEx::showRow(int nRow, bool bShow)
{
    QWidget* label = getRowLabel(nRow);
    QWidget* field = getRowField(nRow);
    QHBoxLayout* hLayout = getRowLayout(nRow);

    if (!label || !field || !hLayout)
        return;

    if (bShow) {
        if (bShow == !label->isHidden())
            insertLayout(nRow,hLayout);
    } else
        removeItem(hLayout);

    label->setVisible(bShow);
    field->setVisible(bShow);
}

GeneralPreviewPluginPrivate::GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent)
    : q_p(parent)
    , m_contentWidget(new QWidget())
{
    // 图标和名称
    m_iconLabel = new QLabel(m_contentWidget);
    m_iconLabel->setObjectName("IconLabel");
    m_iconLabel->setFixedSize(QSize(ICON_SIZE, ICON_SIZE));
    m_nameLabel = new NameLabel("", m_contentWidget);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(HOR_MARGIN_SIZE - 3, MARGIN_SIZE, HOR_MARGIN_SIZE, MARGIN_SIZE);
    hLayout->setSpacing(MARGIN_SIZE * 2);
    hLayout->addWidget(m_iconLabel);
    hLayout->addWidget(m_nameLabel);
    hLayout->setStretch(0,1);
    hLayout->setStretch(0,9);

    // 基本信息
    m_basicInfoLayout = new VBoxLayoutEx();
    m_basicInfoLayout->setContentsMargins(HOR_MARGIN_SIZE, MARGIN_SIZE, HOR_MARGIN_SIZE, MARGIN_SIZE);
    m_basicInfoLayout->setSpacing(LAYOUT_SPACING);

    SectionKeyLabel *label = nullptr;
    SectionValueLabel *value = nullptr;
    for (int i = 0; i < RowCount; i++) {
        label = new SectionKeyLabel(getBasicInfoLabelName(static_cast<BasicInfoRow>(i)));
        value = new SectionValueLabel;
        value->setWordWrap(false);
        m_basicInfoLayout->addRow(label, value);
    }
    m_basicInfoLayout->showRow(ContainSize_Row, false);

    m_vMainLayout = new QVBoxLayout(m_contentWidget);

    m_line1 = new DHorizontalLine;
    m_line1->setFrameShadow(DHorizontalLine::Raised);
    m_line1->setLineWidth(1);

    m_line2 = new DHorizontalLine;
    m_line2->setFrameShadow(DHorizontalLine::Raised);
    m_line2->setLineWidth(1);

    // 动作标签
    m_openLabel = new ActionLabel(QObject::tr("Open"));
    m_openPathLabel= new ActionLabel(QObject::tr("Open path"));
    m_copyPathLabel = new ActionLabel(QObject::tr("Copy path"));
    QVBoxLayout* actionLayout = new QVBoxLayout();
    actionLayout->setContentsMargins(HOR_MARGIN_SIZE, MARGIN_SIZE, MARGIN_SIZE, 0);
    actionLayout->setSpacing(LAYOUT_SPACING);
    actionLayout->addWidget(m_openLabel);
    actionLayout->addWidget(m_openPathLabel);
    actionLayout->addWidget(m_copyPathLabel);

    m_vMainLayout->setContentsMargins(MARGIN_SIZE, 0, MARGIN_SIZE, MARGIN_SIZE);
    m_vMainLayout->addLayout(hLayout);
    m_vMainLayout->addWidget(m_line1);
    m_vMainLayout->addLayout(m_basicInfoLayout);
    m_vMainLayout->addWidget(m_line2);
    m_vMainLayout->addLayout(actionLayout);
    m_vMainLayout->addSpacing(0);
    m_vMainLayout->addStretch();
}

QString GeneralPreviewPluginPrivate::getBasicInfoLabelName(GeneralPreviewPluginPrivate::BasicInfoRow eRow)
{
    QString name = "";

    switch (eRow) {
    case Location_Row:
        name = QObject::tr("Location:");
        break;
    case Size_Row:
        name = QObject::tr("Size:");
        break;
    case ContainSize_Row:
        name = QObject::tr("Contains:");
        break;
    case TimeModified_Row:
        name = QObject::tr("Time modified:");
        break;
    default:
        break;
    }

    return name;
}

void GeneralPreviewPluginPrivate::setBasicInfo(const MatchedItem &item)
{
    Q_ASSERT(m_basicInfoLayout);

    QFileInfo fi(item.item);

    //m_basicInfoLayout->showRow(ContainSize_Row, fi.isDir());

    SET_BASICINFO_VALUE_LABEL_TEXT(Location_Row, fi.absoluteFilePath());
    SET_BASICINFO_VALUE_LABEL_TEXT(Size_Row, Utils::formatFileSize(fi.size()));
    SET_BASICINFO_VALUE_LABEL_TEXT(TimeModified_Row, fi.lastModified().toString(Utils::dateTimeFormat()));
}

GeneralPreviewPlugin::GeneralPreviewPlugin(QObject *parent)
    : PreviewPlugin(parent)
    , d_p(new GeneralPreviewPluginPrivate(this))
{
    d_p->m_contentWidget->installEventFilter(this);
    initConnect();
}

GeneralPreviewPlugin::~GeneralPreviewPlugin()
{
    Q_ASSERT(d_p->m_contentWidget);
    d_p->m_contentWidget->deleteLater();
}

bool GeneralPreviewPlugin::previewItem(const MatchedItem &item)
{
    if (!item.item.isEmpty()
            && !item.name.isEmpty()
            && d_p->m_item.item == item.item
            && d_p->m_item.name == item.name)
        return true;

    d_p->m_item = item;

    QPixmap pixmap;
    const QSize iconSize(ICON_SIZE, ICON_SIZE);
    const QString &strIcon = item.icon;
    if (!strIcon.isEmpty()) {
        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                pixmap = QIcon(strIcon).pixmap(iconSize);
            else
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (icon.isNull())
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
            else
                pixmap = icon.pixmap(iconSize);
        }
    } else {
        pixmap = Utils::defaultIcon(item).pixmap(iconSize);
    }

    d_p->m_iconLabel->setPixmap(pixmap);

    // 计算换行内容
    QString elidedText = lineFeed(item.name, d_p->m_nameLabel->width(), d_p->m_nameLabel->font());
    d_p->m_nameLabel->setText(elidedText);
    if (elidedText != item.name)
        d_p->m_nameLabel->setToolTip(item.name);

    d_p->setBasicInfo(item);

    return true;
}

MatchedItem GeneralPreviewPlugin::item() const
{
    return d_p->m_item;
}

QWidget *GeneralPreviewPlugin::contentWidget() const
{
    return d_p->m_contentWidget;
}

void GeneralPreviewPlugin::onOpenClicked()
{
    Utils::openFile(d_p->m_item);
}

void GeneralPreviewPlugin::onOpenpathClicked()
{
    QFileInfo fi(d_p->m_item.item);
    if (fi.exists()) {
        MatchedItem item = d_p->m_item;
        item.icon = "inode-directory";
        item.item = fi.absolutePath();
        item.name = fi.dir().dirName();
        item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;
        item.type = "inode/directory";

        Utils::openFile(item);
    }
}

void GeneralPreviewPlugin::onCopypathClicked()
{
    QWidget* topWidget = contentWidget()->topLevelWidget();
    Q_ASSERT(topWidget);

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(d_p->m_item.item);

    auto showPoint = topWidget->mapToGlobal(QPoint(topWidget->width() / 2, topWidget->height() / 2 + 40));
    auto color = topWidget->palette().background().color();

    Utils::showAlertMessage(showPoint, color, QObject::tr("Path information has been copied to the clipboard."), 2000);
}

void GeneralPreviewPlugin::initConnect()
{
    Q_ASSERT(d_p->m_openLabel);
    Q_ASSERT(d_p->m_openPathLabel);
    Q_ASSERT(d_p->m_copyPathLabel);

    connect(d_p->m_openLabel, &ActionLabel::sigLabelClicked, this, &GeneralPreviewPlugin::onOpenClicked);
    connect(d_p->m_openPathLabel, &ActionLabel::sigLabelClicked, this, &GeneralPreviewPlugin::onOpenpathClicked);
    connect(d_p->m_copyPathLabel, &ActionLabel::sigLabelClicked, this, &GeneralPreviewPlugin::onCopypathClicked);
}

QString GeneralPreviewPlugin::lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow)
{
    if (nElidedRow < 0)
        nElidedRow = 2;

    QString strText = text;
    int nIndex = 1;
    QFontMetrics fm(font);
    if (!strText.isEmpty()) {
        for (int i = 1; i < strText.size() + 1; i++) {
            if (fm.width(strText.left(i)) > nWidth * nIndex) {
                nIndex++;
                if (nIndex > nElidedRow) {
                    strText = fm.elidedText(strText, Qt::ElideRight, nWidth * nElidedRow);
                    break;
                }
                strText.insert(i - 1, "\n");
            }
        }
    }

    return strText;
}
