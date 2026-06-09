// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchlistdelegate.h"
#include "grandsearchlistview.h"
#include "global/builtinsearch.h"
#include "global/widgets/highlightutils.h"

#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionViewItem>
#include <QTextDocument>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QToolTip>
#include <QFontMetricsF>

#define ListIconSize 24   // 列表图标大小
#define ListRowWidth 740   // 列表行宽
#define ItemDataSpacing 10   // 信息显示间隔
#define TailMaxWidth 150   // 拖尾信息最大显示宽度
#define ContextLineSpacing 1   // 文件名与匹配内容行间距
#define ListLineMargin 8   // 行上下边距

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

GrandSearchListDelegate::GrandSearchListDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

GrandSearchListDelegate::~GrandSearchListDelegate()
{
}

void GrandSearchListDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    DStyledItemDelegate::initStyleOption(option, index);
    option->text = "";
    option->icon = QIcon();
}

bool GrandSearchListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return true;
}

void GrandSearchListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
#endif

    // 清除该行的 tooltip 区域缓存
    m_tooltipRegionCache[index.row()].clear();

    // 让基类绘制背景和选中状态（不绘制图标和文本）
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
    pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

    // 绘制选中状态
    drawSelectState(painter, viewOption, index);

    // 调整右边距10px
    viewOption.rect.adjust(0, 0, -10, 0);

    // 绘制图标（普通图标或缩略图）
    drawIcon(painter, viewOption, index);

    // 绘制匹配结果文本
    drawSearchResultText(painter, viewOption, index);
}

QSize GrandSearchListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    bool hasSecondLine = false;
    if (index.isValid()) {
        QVariant extra = index.data(DATA_ROLE).value<MatchedItem>().extra;
        if (extra.isValid()) {
            const QVariantHash &hash = extra.toHash();
            hasSecondLine = hash.contains(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT)
                    || hash.contains(GRANDSEARCH_PROPERTY_ITEM_TAILER)
                    || hash.contains(GRANDSEARCH_PROPERTY_ITEM_MODIFIED_TIME);
        }
    }

    QFontMetrics nameFm(DFontSizeManager::instance()->get(DFontSizeManager::T6));
    int totalHeight = ListLineMargin * 2 + nameFm.height();

    if (hasSecondLine) {
        QFontMetrics contextFm(DFontSizeManager::instance()->get(DFontSizeManager::T8));
        totalHeight += ContextLineSpacing + contextFm.height();
    }

    return QSize(ListRowWidth, totalHeight);
}

static void hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

bool GrandSearchListDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QPoint itemLocalPos = event->pos();
        const QList<TooltipRegion> &regions = m_tooltipRegionCache.value(index.row());
        for (const TooltipRegion &region : regions) {
            if (region.rect.contains(itemLocalPos)) {
                int tooltipsize = region.fullText.size();
                const int nlong = 32;
                int lines = tooltipsize / nlong + 1;
                QString strtooltip;
                for (int i = 0; i < lines; ++i) {
                    strtooltip.append(region.fullText.mid(i * nlong, nlong));
                    strtooltip.append("\n");
                }
                strtooltip.chop(1);
                QToolTip::showText(event->globalPos(), strtooltip, view, region.rect);
                return true;
            }
        }

        // 鼠标在非省略区域，隐藏 tooltip
        hideTooltipImmediately();
        return true;
    }

    return DStyledItemDelegate::helpEvent(event, view, option, index);
}

void GrandSearchListDelegate::recordTooltipRegion(int row, const QRect &rect, const QString &fullText) const
{
    m_tooltipRegionCache[row].append({ rect, fullText });
}

void GrandSearchListDelegate::clearTooltipCache()
{
    m_tooltipRegionCache.clear();
}

void GrandSearchListDelegate::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant thumbnailData = index.data(THUMBNAIL_ROLE);
    if (thumbnailData.isValid() && !thumbnailData.value<QPixmap>().isNull()) {
        QPixmap thumbnail = thumbnailData.value<QPixmap>();
        // 按实际比例缩放，宽高不超过 ListIconSize
        QSize scaledSize = thumbnail.size().scaled(ListIconSize, ListIconSize, Qt::KeepAspectRatio);
        QRect iconRect(QPoint(), scaledSize);
        iconRect.moveCenter(QPoint(option.rect.x() + ItemDataSpacing + ListIconSize / 2,
                                   option.rect.y() + option.rect.height() / 2));

        painter->save();

        // 圆角裁剪
        QPainterPath clipPath;
        clipPath.addRoundedRect(iconRect, 2, 2);
        painter->setClipPath(clipPath);
        painter->drawPixmap(iconRect, thumbnail);

        // 内描边
        painter->setClipping(false);
        QColor borderColor = option.palette.color(QPalette::BrightText);
        borderColor.setAlphaF(0.1);

        QPen borderPen(borderColor, 1);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(iconRect, 2, 2);
        painter->restore();
    } else {
        // 绘制普通图标
        QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        if (icon.isNull())
            return;

        QRect iconRect(option.rect.x() + ItemDataSpacing, option.rect.y() + (option.rect.height() - ListIconSize) / 2,
                       ListIconSize, ListIconSize);
        painter->save();
        icon.paint(painter, iconRect);
        painter->restore();
    }
}

void GrandSearchListDelegate::drawSelectState(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    if (option.state & QStyle::State_Selected)
        return;

    if (option.state & QStyle::State_MouseOver) {
        painter->save();
        painter->setPen(Qt::NoPen);

        QColor hovertColor;
        const GrandSearchListView *listview = qobject_cast<const GrandSearchListView *>(option.widget);
        if ((listview->getThemeType() == DGuiApplicationHelper::DarkType)
            || (index.isValid() && index == listview->currentIndex())) {
            hovertColor = QColor(255, 255, 255, static_cast<int>(255 * 0.05));
        } else {
            hovertColor = QColor(0, 0, 0, static_cast<int>(255 * 0.05));
        }
        painter->setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(0, 0, 0, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }
}

void GrandSearchListDelegate::drawSearchResultText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const MatchedItem &item = index.data(DATA_ROLE).value<MatchedItem>();
    const QVariantHash &extraHash = item.extra.toHash();

    const GrandSearchListView *listview = qobject_cast<const GrandSearchListView *>(option.widget);
    bool isDark = (listview->getThemeType() == DGuiApplicationHelper::DarkType)
            || (index.isValid() && index == listview->currentIndex());
    QColor nameTextColor = isDark ? QColor("#FFFFFF") : QColor("#000000");
    nameTextColor.setAlpha(255 * 0.9);

    QStringList keywords = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS, QStringList()).toStringList();
    QFont nameFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFontMetrics nameFontMetrics(nameFont);

    QFont secondFont = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    QFontMetrics secondFontMetrics(secondFont);
    int textStartX = ItemDataSpacing + ListIconSize + ItemDataSpacing;
    int firstLineY = option.rect.y() + ListLineMargin;
    int secondLineY = firstLineY + nameFontMetrics.height() + ContextLineSpacing;

    // 第一行：文件名
    drawItemName(painter, index, option, item.name, item.searcher, keywords,
                 nameFont, nameFontMetrics, nameTextColor, isDark, firstLineY);

    // 第二行颜色（修改时间、拖尾共用）
    QColor secondLineColor = isDark ? QColor("#FFFFFF") : QColor("#000000");
    secondLineColor.setAlphaF(0.5);

    bool isPreview = listview->isPreviewItem();
    int currentX = textStartX;
    int remainingWidth = option.rect.width() - currentX;

    // 修改时间（独立字段，不限制宽度）
    QString modifiedTime = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_MODIFIED_TIME).toString();
    if (!modifiedTime.isEmpty()) {
        modifiedTime.prepend(tr("Modified at "));
        drawSecondLineText(painter, modifiedTime, secondFont, secondFontMetrics, secondLineColor, currentX, secondLineY);
        remainingWidth = option.rect.width() - currentX;
    }

    // 拖尾数据（预览时不显示）
    if (!isPreview) {
        QStringList tailList = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_TAILER, QStringList()).toStringList();
        if (!tailList.isEmpty()) {
            if (remainingWidth > ItemDataSpacing) {
                currentX += ItemDataSpacing;
                remainingWidth = option.rect.width() - currentX;
            }
            currentX = drawTailText(painter, index, tailList, secondFont, secondFontMetrics,
                                    secondLineColor, remainingWidth, currentX, secondLineY);
            remainingWidth = option.rect.width() - currentX;
        }
    }

    // 匹配摘要
    if (!isPreview && remainingWidth > ItemDataSpacing) {
        QString context = extraHash.value(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT).toString();
        if (!context.isEmpty()) {
            currentX += ItemDataSpacing;
            drawMatchedContext(painter, index, option, context, keywords,
                               secondFont, secondFontMetrics, isDark, secondLineY, currentX);
        }
    }
}

void GrandSearchListDelegate::drawItemName(QPainter *painter, const QModelIndex &index, const QStyleOptionViewItem &option,
                                           const QString &name, const QString &searcher,
                                           const QStringList &keywords, const QFont &font,
                                           const QFontMetrics &fontMetrics, const QColor &textColor,
                                           bool isDark, int startY) const
{
    int textStartX = ItemDataSpacing + ListIconSize + ItemDataSpacing;
    int nameTextMaxWidth = option.rect.width() - textStartX;

    // 使用 QFontMetricsF 进行精确的字体宽度计算
    QFontMetricsF fontMetricsF(fontMetrics);

    // 使用智能省略，确保高亮关键词优先显示
    HighlightUtils::ElideResult elideResult;

    if (!keywords.isEmpty()) {
        auto matchRanges = HighlightUtils::findMatchRanges(name, keywords);
        if (!matchRanges.isEmpty()) {
            elideResult = HighlightUtils::smartElideWithTracking(name, nameTextMaxWidth, fontMetricsF, matchRanges);
        }
    }
    if (elideResult.text.isEmpty())
        elideResult = HighlightUtils::elideWithTracking(name, Qt::ElideRight, nameTextMaxWidth, fontMetricsF);

    QString displayText = elideResult.text;

    // Web 搜索器特殊处理：保留末尾的标记字符
    if (elideResult.text != name && GRANDSEARCH_CLASS_WEB_STATICTEXT == searcher) {
        QString lastChar = name.right(1);
        int lastCharWidth = static_cast<int>(fontMetricsF.horizontalAdvance(lastChar));
        nameTextMaxWidth -= lastCharWidth;
        if (nameTextMaxWidth > 0) {
            elideResult = HighlightUtils::elideWithTracking(name.left(name.size() - 1), Qt::ElideRight, nameTextMaxWidth, fontMetricsF);
            displayText = elideResult.text + lastChar;
        }
    }

    // 构建高亮格式（映射到省略后的文本位置）
    QTextCharFormat hlFmt;
    hlFmt.setFontWeight(QFont::DemiBold);
    hlFmt.setForeground(isDark ? QColor("#FFFFFF") : QColor("#000000"));
    auto formats = HighlightUtils::buildFormatRanges(
            displayText, elideResult.origPositions, name, keywords, hlFmt);

    // 使用 QTextLayout 绘制
    QTextLayout layout;
    layout.setText(displayText);
    layout.setFont(font);
    layout.setFormats(formats);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();

    QRect drawRect(textStartX, startY, nameTextMaxWidth, fontMetrics.height());

    // 文件名被省略时，记录 tooltip 区域
    if (elideResult.text != name) {
        recordTooltipRegion(index.row(), drawRect, name);
    }

    painter->save();
    painter->translate(textStartX, startY);
    painter->setPen(textColor);
    layout.draw(painter, QPointF(0, 0));
    painter->restore();
}

void GrandSearchListDelegate::drawMatchedContext(QPainter *painter, const QModelIndex &index, const QStyleOptionViewItem &option,
                                                 const QString &matchedContext, const QStringList &keywords,
                                                 const QFont &font, const QFontMetrics &fontMetrics,
                                                 bool isDark, int startY, int startX) const
{
    if (startX <= 0)
        startX = ItemDataSpacing + ListIconSize + ItemDataSpacing;
    int maxWidth = option.rect.width() - startX;

    // 使用 QFontMetricsF 进行精确的字体宽度计算
    QFontMetricsF fontMetricsF(fontMetrics);

    QColor textColor = isDark ? QColor(255, 255, 255, int(255 * 0.5)) : QColor(0, 0, 0, int(255 * 0.5));
    QColor highlightColor = isDark ? QColor(255, 255, 255, int(255 * 0.8)) : QColor(0, 0, 0, int(255 * 0.85));

    // 以最左边的关键词匹配为锚点进行省略
    auto allRanges = HighlightUtils::findMatchRanges(matchedContext, keywords);
    QVector<HighlightUtils::MatchRange> leftmostOnly;
    if (!allRanges.isEmpty())
        leftmostOnly.append(allRanges.constFirst());

    HighlightUtils::ElideResult elideResult;
    if (!leftmostOnly.isEmpty())
        elideResult = HighlightUtils::smartElideWithTracking(matchedContext, maxWidth, fontMetricsF, leftmostOnly);
    if (elideResult.text.isEmpty())
        elideResult = HighlightUtils::elideWithTracking(matchedContext, Qt::ElideRight, maxWidth, fontMetricsF);

    // 构建高亮格式
    QTextCharFormat hlFmt;
    hlFmt.setFont(font);
    hlFmt.setFontWeight(QFont::DemiBold);
    hlFmt.setForeground(highlightColor);
    auto formats = HighlightUtils::buildFormatRanges(
            elideResult.text, elideResult.origPositions, matchedContext, keywords, hlFmt);

    // 绘制
    QTextLayout layout;
    layout.setText(elideResult.text);
    layout.setFont(font);
    layout.setFormats(formats);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();

    painter->save();
    painter->setPen(textColor);
    painter->translate(startX, startY);
    layout.draw(painter, QPointF(0, 0));
    painter->restore();

    // 摘要被省略时，记录 tooltip 区域
    if (elideResult.text != matchedContext) {
        QTextLine tl = layout.lineAt(0);
        recordTooltipRegion(index.row(),
                            QRect(startX, startY, int(tl.naturalTextWidth()), fontMetrics.height()),
                            matchedContext);
    }
}

int GrandSearchListDelegate::drawTailText(QPainter *painter, const QModelIndex &index, const QStringList &tailList,
                                          const QFont &font, const QFontMetrics &fontMetrics, const QColor &color,
                                          int maxWidth, int startX, int startY) const
{
    if (tailList.isEmpty())
        return startX;

    int tailCount = tailList.count();
    int tailAverageWidth = maxWidth;
    calcTailShowInfo(maxWidth, tailCount, tailAverageWidth);

    QStringList mutableList = tailList;
    const QMap<int, QString> &tailMap = calcTailShowData(mutableList, tailCount, tailAverageWidth, fontMetrics);
    if (tailMap.isEmpty())
        return startX;

    int currentX = startX;
    for (auto tailIndex : tailMap.keys()) {
        if (tailIndex != 0)
            currentX += ItemDataSpacing;
        const QString &elidedTail = tailMap.value(tailIndex);

        // 拖尾文本被截断时，记录 tooltip 区域
        if (elidedTail != tailList.value(tailIndex)) {
            int tailWidth = fontMetrics.size(Qt::TextSingleLine, elidedTail).width();
            recordTooltipRegion(index.row(),
                                QRect(currentX, startY, tailWidth, fontMetrics.height()),
                                tailList.value(tailIndex));
        }
        drawSecondLineText(painter, elidedTail, font, fontMetrics, color, currentX, startY);
    }

    return currentX;
}

void GrandSearchListDelegate::drawSecondLineText(QPainter *painter, const QString &text, const QFont &font,
                                                 const QFontMetrics &fontMetrics, const QColor &color,
                                                 int &startX, int startY) const
{
    int showWidth = fontMetrics.size(Qt::TextSingleLine, text).width();

    QTextLayout layout;
    layout.setText(text);
    layout.setFont(font);
    QTextCharFormat fmt;
    fmt.setForeground(color);
    layout.setFormats({ QTextLayout::FormatRange { 0, static_cast<int>(text.length()), fmt } });

    layout.beginLayout();
    layout.createLine();
    layout.endLayout();

    painter->save();
    painter->translate(startX, startY);
    layout.draw(painter, QPointF(0, 0));
    painter->restore();

    startX += showWidth;
}

void GrandSearchListDelegate::calcTailShowInfo(const int totalTailWidth, int &tailCount, int &averageWidth) const
{
    Q_ASSERT(tailCount >= 1);

    while (1 != tailCount) {
        // 计算每个拖尾信息平均可显示宽度(每个拖尾信息之间显示间隔10个像素)
        averageWidth = (totalTailWidth - (tailCount - 1) * ItemDataSpacing) / tailCount;
        if (averageWidth > 0)
            break;
        // 宽度不够显示 (tailCount - 1) * TailDataMargin 个间隔，则减少拖尾数量继续计算
        tailCount--;
        averageWidth = totalTailWidth;
    }
}

QMap<int, QString> GrandSearchListDelegate::calcTailShowData(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const
{
    if (averageWidth >= TailMaxWidth) {
        averageWidth = TailMaxWidth;

        // 平均宽度大于最大限制，则每个项可用的最大宽度均为限制宽度，此时不需要计算剩余空间，再累加给截断显示项进行使用
        return calcTailShowDataByMaxWidth(strings, tailCount, averageWidth, fontMetrics);
    }

    // 平均宽度小于最大限制，则需要针对每个项，计算使用后的剩余空间，并累加后提供给被截断显示的项使用
    return calcTailShowDataByOptimalWidth(strings, tailCount, averageWidth, fontMetrics);
}

QMap<int, QString> GrandSearchListDelegate::calcTailShowDataByMaxWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const
{
    QString tailString;   // 拖尾数据
    QString elidedTailText;   // 实际显示的拖尾数据
    QMap<int, QString> tailMap;   // 最终显示的数据--<拖尾序号，拖尾数据>

    for (int i = 0; i < tailCount; i++) {
        tailString = strings.takeFirst();
        elidedTailText = fontMetrics.elidedText(tailString, Qt::ElideMiddle, averageWidth);
        if (!elidedTailText.isEmpty())
            tailMap.insert(i, elidedTailText);
    }

    return tailMap;
}

QMap<int, QString> GrandSearchListDelegate::calcTailShowDataByOptimalWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const
{
    QString tailString;   // 拖尾数据
    QMap<int, QString> tailMap;   // 最终显示的数据--<拖尾序号，拖尾数据>
    QMap<int, QString> pendMap;   // 推迟计算的拖尾数据(仅使用平均空间时会发生截断的拖尾数据)
    int unUsedWidth = 0;   // 未使用空间
    // 提取能够全量显示的拖尾信息
    for (int i = 0; i < tailCount; i++) {
        // 从前向后，逐个取出数据进行计算
        tailString = strings.takeFirst();
        int currentTailWidth = fontMetrics.size(Qt::TextSingleLine, tailString).width();
        if (currentTailWidth < averageWidth) {
            // 该项拖尾数据能够全量显示，则保存，并累加剩余空间
            unUsedWidth += averageWidth - currentTailWidth;
            if (!tailString.isEmpty())
                tailMap.insert(i, tailString);
            continue;
        }
        pendMap.insert(i, tailString);
    }

    // 提取被截断显示的拖尾信息
    int availableWidth;   // 当前项可利用的最大宽度
    QString pendString;   // 待处理的原始信息
    QString elidedTailText;   // 截断显示的拖尾数据
    for (auto index : pendMap.keys()) {

        availableWidth = averageWidth;
        if (unUsedWidth > 0) {
            availableWidth += unUsedWidth;
            unUsedWidth = 0;

            if (availableWidth > TailMaxWidth) {
                // 退还超出限制的空间
                unUsedWidth = availableWidth - TailMaxWidth;
                availableWidth = TailMaxWidth;
            }
        }

        pendString = pendMap.value(index);
        elidedTailText = fontMetrics.elidedText(pendString, Qt::ElideMiddle, availableWidth);
        if (elidedTailText == pendString) {
            // 未截断显示，说明设置的可用空间还有剩余，需要退还给未使用空间
            int elidedTailWidth = fontMetrics.size(Qt::TextSingleLine, pendString).width();
            unUsedWidth += availableWidth - elidedTailWidth;
        }

        if (!elidedTailText.isEmpty())
            tailMap.insert(index, elidedTailText);
        pendMap.remove(index);
    }

    return tailMap;
}
