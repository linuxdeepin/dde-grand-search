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

#include <QFileInfo>
#include <QDateTime>
#include <QtConcurrent>
#include <QPainterPath>

#ifdef __cplusplus
extern "C" {
#include <libavformat/avformat.h>
#include <libffmpegthumbnailer/videothumbnailerc.h>
}
#endif

namespace  {
static const QString kLabelDimension = QObject::tr("Dimension:");
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
    if (!m_future.isFinished()) {
        stopPreview();
        m_future.waitForFinished();
    }
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

bool VideoPreviewPlugin::previewItem(const GrandSearch::ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    //开启线程解析
    m_decoding = true;
#ifdef PREVIEW_ASYNC_DECODE
    m_future = QtConcurrent::run(&VideoPreviewPlugin::decode, path, this, true);
#else
    m_future = QtConcurrent::run(&VideoPreviewPlugin::decode, path, this, false);
#endif

    //初始化静态属性
    QFileInfo fileInfo(path);
    m_infos.clear();
    m_infos.append(DetailInfo(kLabelDimension, QString("--")));
    m_infos.append(DetailInfo(kLabelType, fileInfo.suffix().toUpper()));
    m_infos.append(DetailInfo(kLabelSize, GrandSearch::CommonTools::formatFileSize(fileInfo.size())));
    m_infos.append(DetailInfo(kLabelDuration, QString("--")));
    m_infos.append(DetailInfo(kLabelLocation, fileInfo.absolutePath()));
    {
        auto lt = fileInfo.lastModified().toString(GrandSearch::CommonTools::dateTimeFormat());
        m_infos.append(DetailInfo(kLabelTime, lt.isEmpty() ? QString("--") : lt));
    }

    m_view->setTitle(fileInfo.fileName());
    m_item = item;

#ifndef PREVIEW_ASYNC_DECODE
    //同步更新，防止闪烁。若解析时间过长再调整为异步
    m_future.waitForFinished();
    updateInfo(m_future.result(), false);
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
    m_decoding = false;
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

void VideoPreviewPlugin::updateInfo(const QVariantHash &hash, bool needUpdate)
{
    bool updateDetail = false;
    if (hash.contains(kLabelDimension)) {
        QSize size = hash.value(kLabelDimension).toSize();
        auto org = m_infos.takeAt(kDimensionIndex);
        Q_ASSERT(org.first == kLabelDimension);
        m_infos.prepend(DetailInfo(kLabelDimension, QString("%0*%1").arg(size.width()).arg(size.height())));
        updateDetail = true;
    }

    if (hash.contains(kLabelDuration) && m_view) {
        auto duration = hash.value(kLabelDuration).value<qint64>();
        auto org = m_infos.takeAt(kDurationIndex);
        Q_ASSERT(org.first == kLabelDuration);
        m_infos.insert(kDurationIndex, DetailInfo(kLabelDuration, GrandSearch::CommonTools::durationString(duration)));
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

QVariantHash VideoPreviewPlugin::decode(const QString &file, VideoPreviewPlugin *self, bool updateBySignal)
{
    if (!self || !self->m_decoding)
        return {};

    QVariantHash info;

    //获取分辨率和时长
    AVFormatContext *avCtx = nullptr;
    qint64 duration = 0;
    auto stdStr = file.toStdString();
    if (avformat_open_input(&avCtx, stdStr.c_str(), nullptr , nullptr) == 0) {
        int videoRet = av_find_best_stream(avCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (videoRet >= 0) {
            AVStream *videoStream = avCtx->streams[videoRet];
            AVCodecParameters *codecpar = videoStream->codecpar;
            duration = avCtx->duration / (qint64)AV_TIME_BASE;
            info.insert(kLabelDuration, QVariant::fromValue(duration));
            info.insert(kLabelDimension, QSize(codecpar->width, codecpar->height));
        } else {
            qWarning() << "VideoPreviewPlugin: find stream error" << videoRet;
        }

        avformat_close_input(&avCtx);
    } else {
        qWarning() << "VideoPreviewPlugin: could not open video....";
    }

    //检查一次是否停止
    if (!self->m_decoding)
        return {};

    //时长大于0才获取预览图
    if (duration > 0) {
        //获取预览图
        video_thumbnailer *thumbnailer = video_thumbnailer_create();
        //缩略图最大size
         auto maxSize = VideoView::maxThumbnailSize();
         thumbnailer->thumbnail_size = qMax(maxSize.width(), maxSize.height());

        //第一秒
        thumbnailer->seek_time = const_cast<char *>("00:00:01");

        image_data *imageData = video_thumbnailer_create_image_data();
        if (video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, stdStr.c_str(), imageData) == 0) {
            QImage img = QImage::fromData(imageData->image_data_ptr,
                                          static_cast<int>(imageData->image_data_size), "png");
            //缩放与圆角处理
            QPixmap pixmap = VideoPreviewPlugin::scaleAndRound(img, maxSize);
            info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
        } else {
            // 预览失败
            qWarning() << "thumbnailer create image error";
            QImage errorImg(":/icons/damaged.svg");
            errorImg = errorImg.scaled(46, 46);
            auto img = GrandSearch::CommonTools::creatErrorImage({192, 108}, errorImg);
            QPixmap pixmap = VideoPreviewPlugin::scaleAndRound(img, maxSize);
            info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
        }
        video_thumbnailer_destroy_image_data(imageData);
        video_thumbnailer_destroy(thumbnailer);
    } else {
        // 预览失败
        QImage errorImg(":/icons/damaged.svg");
        errorImg = errorImg.scaled(46, 46);
        auto img = GrandSearch::CommonTools::creatErrorImage({192, 108}, errorImg);
        QPixmap pixmap = VideoPreviewPlugin::scaleAndRound(img, VideoView::maxThumbnailSize());
        info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
    }

    //检查一次是否中断
    if (!self->m_decoding)
        return {};

    self->m_decoding = false;
    if (updateBySignal)
        QMetaObject::invokeMethod(self, "updateInfo", Qt::QueuedConnection
                                  , Q_ARG(QVariantHash, info)
                                  , Q_ARG(bool, true));

    return info;
}

QPixmap VideoPreviewPlugin::scaleAndRound(const QImage &img, const QSize &size)
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
