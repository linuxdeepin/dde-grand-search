// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIGHLIGHTLABEL_H
#define HIGHLIGHTLABEL_H

#include <QLabel>
#include <QTextLayout>

class HighlightLabel : public QLabel
{
    Q_OBJECT
public:
    enum class ElideMode {
        None    = Qt::ElideNone,
        Left    = Qt::ElideLeft,
        Right   = Qt::ElideRight,
        Middle  = Qt::ElideMiddle,
        Smart   = 5
    };
    Q_ENUM(ElideMode)

    explicit HighlightLabel(QWidget *parent = nullptr, Qt::WindowFlags f = {});

    void setPlainText(const QString &text);
    void setKeywords(const QStringList &keywords);
    void setElideMode(ElideMode mode);
    void setMaxLines(int lines);

    ElideMode elideMode() const;
    int maxLines() const;
    bool isElided() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    struct ElideInfo {
        QString displayText;
        QVector<int> origPosMapping; // display char -> original char position, empty = identity
        bool elided = false;
    };

    void relayout();
    void doLayout();
    ElideInfo computeElidedText(int layoutWidth) const;
    ElideInfo computeSmartElidedText(const QString &text, int maxWidth, const QFontMetrics &fm) const;
    void buildFinalLayout(const QString &displayText, const QVector<int> &origPosMapping, int layoutWidth);
    QVector<QTextLayout::FormatRange> buildFormatRanges(
        const QString &displayText,
        const QVector<int> &origPosMapping = {}) const;

    QString m_text;
    QStringList m_keywords;
    ElideMode m_elideMode = ElideMode::Right;
    int m_maxLines = 1;
    QTextLayout m_layout;
    QTextOption m_textOption;
    bool m_elided = false;
};

#endif // HIGHLIGHTLABEL_H
