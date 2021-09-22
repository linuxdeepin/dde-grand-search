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

#ifndef COMMONTOOLS_H
#define COMMONTOOLS_H

#include <QStringList>
#include <QPoint>
#include <QColor>
#include <QtDebug>
#include <QImage>
#include <QPainter>

namespace GrandSearch {

namespace CommonTools
{

// 规范化浮点型字串，去掉小数点后多余的0
inline QString normalizeDoubleString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

// 根据文件size换算为KB\MB\GB\TB
inline QString formatFileSize(qint64 num, bool withUnitVisible = true, int precision = 1, int forceUnit = -1,
                              QStringList unitList = QStringList({" B", " KB", " MB", " GB", " TB"}))
{
    if (num < 0) {
        qWarning() << "Negative number passed to formatSize():" << num;
        num = 0;

        return QString::number(num);
    }

    bool isForceUnit = (forceUnit >= 0);

    qreal fileSize(num);
    QStringListIterator i(unitList);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024 && !isForceUnit) {
            break;
        }

        if (isForceUnit && index == forceUnit) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    QString unitString = withUnitVisible ? unit : QString();
    return QString("%1%2").arg(normalizeDoubleString(QString::number(fileSize, 'f', precision)), unitString);
}

// 日期格式
inline QString dateTimeFormat()
{
    return "yyyy/MM/dd HH:mm";
}

inline QString durationString(qint64 seconds)
{
    int hour = static_cast<int>(seconds / 3600);

    QString mmStr = QString("%1").arg(seconds % 3600 / 60, 2, 10, QLatin1Char('0'));
    QString ssStr = QString("%1").arg(seconds % 60, 2, 10, QLatin1Char('0'));

    if (hour > 0) {
        return QString("%1:%2:%3").arg(hour).arg(mmStr).arg(ssStr);
    } else {
        return QString("%1:%2").arg(mmStr).arg(ssStr);
    }
}

inline QImage creatErrorImage(const QSize &imgSize, const QImage &errorImg)
{
    QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(0, 0, img);

    // errorImg居中
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage((img.width() - errorImg.width())/2,
                      (img.height() - errorImg.height())/2, errorImg);
    painter.end();

    return img;
}

}   // end CommonTools

}   // end GrandSearch

#endif // COMMONTOOLS_H
