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

    //??????????????????
#ifdef PREVIEW_ASYNC_DECODE
    m_decode.reset(new DecodeBridge);
    m_decode->decoding = true;
    connect(m_decode.get(), &DecodeBridge::sigUpdateInfo, this, &VideoPreviewPlugin::updateInfo);
    QtConcurrent::run(&DecodeBridge::decode, m_decode, path);
#else
    QFuture<QVariantHash> future = QtConcurrent::run(&DecodeBridge::decode, nullptr, path);
#endif

    //?????????????????????
    QFileInfo fileInfo(path);
    m_infos.clear();

    // ??????
    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelDimension));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(QString("--")));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // ??????
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelType));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.suffix().toUpper()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // ??????
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelSize));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(GrandSearch::CommonTools::formatFileSize(fileInfo.size())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // ??????
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelDuration));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(QString("--")));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // ??????
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(kLabelLocation));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.absolutePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_infos.push_back(detailInfo);

    // ????????????
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
    //?????????????????????????????????????????????????????????????????????
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

    //????????????????????????
    if (needUpdate && updateDetail && m_proxy)
        requestUpdateDetailInfo(m_proxy, this);
}

QVariantHash DecodeBridge::decode(QSharedPointer<DecodeBridge> self, const QString &file)
{
    if (!self.isNull() && !self->decoding)
        return {};

    QVariantHash info;

    //????????????????????????
    AVFormatContext *avCtx = nullptr;
    qint64 duration = 0;
    auto stdStr = file.toStdString();
    if (avformat_open_input(&avCtx, stdStr.c_str(), nullptr , nullptr) == 0) {
        if (avformat_find_stream_info(avCtx, nullptr) >= 0) {
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
        }

        avformat_close_input(&avCtx);
    } else {
        qWarning() << "VideoPreviewPlugin: could not open video....";
    }

    //????????????????????????
    if (!self.isNull() && !self->decoding)
        return {};

    //????????????0??????????????????
    if (duration > 0) {
        //???????????????
        video_thumbnailer *thumbnailer = video_thumbnailer_create();
        //???????????????size
         auto maxSize = VideoView::maxThumbnailSize();
         thumbnailer->thumbnail_size = qMax(maxSize.width(), maxSize.height());

        //?????????
        thumbnailer->seek_time = const_cast<char *>("00:00:01");

        image_data *imageData = video_thumbnailer_create_image_data();
        if (video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, stdStr.c_str(), imageData) == 0) {
            QImage img = QImage::fromData(imageData->image_data_ptr,
                                          static_cast<int>(imageData->image_data_size), "png");
            //?????????????????????
            QPixmap pixmap = DecodeBridge::scaleAndRound(img, maxSize);
            info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
        } else {
            // ????????????
            qWarning() << "thumbnailer create image error";
            QImage errorImg(":/icons/image_damaged.svg");
            errorImg = errorImg.scaled(46, 46);
            auto img = GrandSearch::CommonTools::creatErrorImage({192, 108}, errorImg);
            QPixmap pixmap = DecodeBridge::scaleAndRound(img, maxSize);
            info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
        }
        video_thumbnailer_destroy_image_data(imageData);
        video_thumbnailer_destroy(thumbnailer);
    } else {
        // ????????????
        QImage errorImg(":/icons/image_damaged.svg");
        errorImg = errorImg.scaled(46, 46);
        auto img = GrandSearch::CommonTools::creatErrorImage({192, 108}, errorImg);
        QPixmap pixmap = DecodeBridge::scaleAndRound(img, VideoView::maxThumbnailSize());
        info.insert(kKeyThumbnailer, QVariant::fromValue(pixmap));
    }

    //????????????????????????
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
    // ??????
    pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //?????????
    QPixmap destImage(pixmap.size());
    destImage.fill(Qt::transparent);

    {
        QPainter painter(&destImage);
        painter.setRenderHints(QPainter::Antialiasing, true);
        painter.setRenderHints(QPainter::SmoothPixmapTransform, true);

        // ????????????????????????
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
