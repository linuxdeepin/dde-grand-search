/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef HYPERLINKLABEL_H
#define HYPERLINKLABEL_H

#include <QLabel>
#include <QFontMetrics>
#include <QRegion>

class HyperlinkLabel : public QLabel
{
    Q_OBJECT
public:
    explicit HyperlinkLabel(const QString &leadintText, const QString &hyperlink, const QString &endText, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    QRegion drawText(QString drawText, int &startX, int &startY, int &curRow);
    int displayLength(const int startX, QString showTxt);

private:
    QString m_leadingText;        // 超链接之前的说明字符
    QString m_hyperlink;          // 超链接
    QString m_endText;            // 超链接之后的说明字符
    QString m_showText;           // leadingText + hyperlink + endText
    QStringList m_showTextList;   // 列表保存所有字符

    QFontMetrics m_fontMetrics;   // 绘制字符的字体属性
    QRegion m_hyperlinkRegion;    // 超链接的绘制区域
};

#endif // HYPERLINKLABEL_H
