/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "videopreviewplugin.h"
#include "videoview.h"
#include "global/commontools.h"
#include "libvideoviewer.h"

#include <QFileInfo>
#include <QDateTime>
#include <QtConcurrent>
#include <QPainterPath>

namespace  {
static const QString kLabelDimension = QObject::tr("Dimensions:");
static const QString kLabelType = QObject::tr("Type:");
static const QString kLabelSize = QObject::tr("Size:");
static const QString kLabelDuration = QObject::tr("Duration:");
static const QString kLabelLocation = QObject::tr("Location:");
static const QString kLabelTime = QObject::tr("Time modified:");

static const int kDimensionIndex = 0;
static const int kDurationIndex = 3;

static const QString kKeyThumbnailer = "Thumbnailer";
}

using namespace GrandSearch;

VideoPreviewPlugin::VideoPreviewPlugin(QObject *parent)
    : QObject(parent)
    , GrandSearch::PreviewPlugin()
{

}

VideoPreviewPlugin::~VideoPreviewPlugin()
{
    stopPreview();
    delete m_view;
}

void VideoPreviewPlugin::init(QObject *proxyInter)
{
    m_proxy = proxyInter;
    if (!m_view) {
        m_view = new VideoView();
        m_view->initUI();
    }
}
#define PREVIEW_ASYNC_DECODE
bool VideoPreviewPlugin::previewItem(const GrandSearch::ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    //开启线程解析
#ifdef PREVIEW_ASYNC_DECODE
    m_decode.reset(new DecodeBridge);
    m_decode->decoding = true;
    connect(m_decode.get(), &DecodeBridge::sigUpdateInfo, this, &VideoPreviewPlugin::updateInfo);
    QtConcurrent::run(&DecodeBridge::decode, m_decode, path, m_parser);
#else
    QFuture<QVariantHash> future = QtConcurrent::run(&DecodeBridge::decode, nullptr, path, m_parser);
#endif

    //初始化静态属性
    QFileInfo fileInfo(path);
    m_infos.clear();

    // 尺寸
    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelDimension));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(QString("--")));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // 类型
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelType));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.suffix().toUpper()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // 大小
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelSize));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(GrandSearch::CommonTools::formatFileSize(fileInfo.size())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // 时长
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelDuration));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(QString("--")));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // 位置
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelLocation));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.absolutePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // 修改时间
    auto lt = fileInfo.lastModified().toString(GrandSearch::CommonTools::dateTimeFormat());

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelTime));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(lt.isEmpty() ? QString("--") : lt));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    m_view->setTitle(fileInfo.fileName());
    m_item = item;

#ifndef PREVIEW_ASYNC_DECODE
    //同步更新，防止闪烁。若解析时间过长再调整为异步
    future.waitForFinished();
    updateInfo(future.result(), false);
#endif
    return true;
}

GrandSearch::ItemInfo VideoPreviewPlugin::item() const
{
    return m_item;
}

QWidget *VideoPreviewPlugin::contentWidget() const
{
    return m_view;
}

bool VideoPreviewPlugin::stopPreview()
{
    if (!m_decode.isNull())
        m_decode->decoding = false;
    return true;
}

QWidget *VideoPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool VideoPreviewPlugin::showToolBar() const
{
    return true;
}

GrandSearch::DetailInfoList VideoPreviewPlugin::getAttributeDetailInfo() const
{
    return m_infos;
}

void VideoPreviewPlugin::setParser(QSharedPointer<LibVideoViewer> parser)
{
    m_parser = parser;
}

void VideoPreviewPlugin::updateInfo(const QVariantHash &hash, bool needUpdate)
{
    bool updateDetail = false;
    if (hash.contains(kLabelDimension)) {
        QSize size = hash.value(kLabelDimension).toSize();
        auto org = m_infos.takeAt(kDimensionIndex);
        Q_ASSERT(org.first.value(DetailInfoProperty::Text).toString() == kLabelDimension);

        DetailTagInfo tagInfos;
        tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelDimension));
        tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

        DetailContentInfo contentInfos;
        contentInfos.insert(DetailInfoProperty::Text, QVariant(QString("%0*%1").arg(size.width()).arg(size.height())));
        contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

        DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
        m_infos.prepend(detailInfo);
        updateDetail = true;
    }

    if (hash.contains(kLabelDuration) && m_view) {
        auto duration = hash.value(kLabelDuration).value<qint64>();
        auto org = m_infos.takeAt(kDurationIndex);
        Q_ASSERT(org.first.value(DetailInfoProperty::Text).toString() == kLabelDuration);

        DetailTagInfo tagInfos;
        tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelDuration));
        tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

        DetailContentInfo contentInfos;
        contentInfos.insert(DetailInfoProperty::Text, QVariant(GrandSearch::CommonTools::durationString(duration)));
        contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

        DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
        m_infos.insert(kDurationIndex, detailInfo);
        updateDetail = true;
    }

    if (hash.contains(kKeyThumbnailer) && m_view) {
        m_view->setThumbnail(hash.value(kKeyThumbnailer).value<QPixmap>());
        m_view->repaint();
    }

    //使用代理更新详情
    if (needUpdate && updateDetail && m_proxy)
        requestUpdateDetailInfo(m_proxy, this);
}

QVariantHash DecodeBridge::decode(QSharedPointer<DecodeBridge> self, const QString &file, QSharedPointer<GrandSearch::LibVideoViewer> parser)
{
    if (!self.isNull() && !self->decoding)
        return {};

    Q_ASSERT(parser.get());

    QVariantHash info;

    //获取分辨率和时长
    qint64 duration = 1;
    QSize dimension;
    if (!parser->getMovieInfo(QUrl::fromLocalFile(file), dimension, duration)) {
        qWarning() << "get video info failed" << dimension << duration;
    } else {
        qDebug() << "video duration" << duration << "dimension" << dimension;
        if (duration > 0)
            info.insert(kLabelDuration, QVariant::fromValue(duration));

        if (dimension.isValid())
            info.insert(kLabelDimension, dimension);
    }

    //检查一次是否停止
    if (!self.isNull() && !self->decoding)
        return {};

    // 预览失败
    auto failPreview = [](QVariantHash &info) {
        QImage errorImg(":/icons/image_damaged.svg");
        errorImg = errorImg.scaled(46, 46);
        auto img = GrandSearch::CommonTools::creatErrorImage({192, 108}, errorImg);
        QPixmap pixmap = DecodeBridge::scaleAndRound(img, VideoView::maxThumbnailSize());
        info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
    };

    //时长大于0才获取预览图
    if (duration > 0) {
        //获取预览图
        QImage image;
        parser->getMovieCover(QUrl::fromLocalFile(file), image);
        if (!image.isNull()) {
            QPixmap pixmap = DecodeBridge::scaleAndRound(image, VideoView::maxThumbnailSize());
            info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
        } else {
            // 预览失败
            qWarning() << "LibViewer create image error";
            failPreview(info);
        }
    } else {
        failPreview(info);
    }

    //检查一次是否中断
    if (!self.isNull() && !self->decoding)
        return {};

    if (!self.isNull()) {
        self->decoding = false;
        emit self->sigUpdateInfo(info, true);
    }

    return info;
}

QPixmap DecodeBridge::scaleAndRound(const QImage &img, const QSize &size)
{
    auto pixmap = QPixmap::fromImage(img);
    // 缩放
    pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //空图片
    QPixmap destImage(pixmap.size());
    destImage.fill(Qt::transparent);

    {
        QPainter painter(&destImage);
        painter.setRenderHints(QPainter::Antialiasing, true);
        painter.setRenderHints(QPainter::SmoothPixmapTransform, true);

        // 将图片裁剪为圆角
        QPainterPath path;
        QRect rect(0, 0, destImage.width(), destImage.height());
        path.addRoundedRect(rect, 8, 8);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, destImage.width(), destImage.height(), pixmap);
    }

    return destImage;
}

DecodeBridge::DecodeBridge() : QObject()
{
}

DecodeBridge::~DecodeBridge()
{
}
