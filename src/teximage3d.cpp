/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free 
** Software Foundation and appearing in the file LICENSE.GPL included in 
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "teximage3d_p.h"
#include "canvas3dcommon_p.h"

#include <QJSValueIterator>

/*!
 * \qmltype TextureImage
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains a texture image.
 *
 * An uncreatable QML type that contains a texture image created by calling
 * TextureImageLoader::loadImage().
 *
 * \sa TextureImageLoader
 */

/*!
 * \internal
 */
CanvasTextureImage::CanvasTextureImage(QObject *parent) :
    CanvasAbstractObject(parent),
    m_requestId(0),
    m_state(INITIALIZED),
    m_errorString(""),
    m_pixelCache(0),
    m_pixelCacheFormat(CanvasContext::NONE),
    m_pixelCacheFlipY(false)
{
    m_networkAccessManager = new QNetworkAccessManager(this);
    QObject::connect(m_networkAccessManager, &QNetworkAccessManager::finished,
                     this, &CanvasTextureImage::handleReply);
}

/*!
 * \internal
 */
CanvasTextureImage::~CanvasTextureImage()
{
    delete m_networkAccessManager;
    delete m_pixelCache;
}

/*!
 * \qmlproperty url TextureImage::source()
 * Contains the url to the image.
 */
/*!
 * \internal
 */
const QUrl &CanvasTextureImage::source() const
{
    return m_source;
}

/*!
 * \internal
 */
void CanvasTextureImage::setSource(const QUrl &url)
{
    if (url == m_source)
        return;

    m_source = url;
    emit sourceChanged(m_source);

    load();
}

/*!
 * \qmlmethod int TextureImage::id()
 * Contains the object id.
 */
/*!
 * \internal
 */
ulong CanvasTextureImage::id()
{
    return ulong(this);
}

/*!
 * \internal
 */
void CanvasTextureImage::load()
{
    if (m_source.isEmpty()) {
        QByteArray array;
        m_image.loadFromData(array);
        m_glImage = m_image.convertToFormat(QImage::Format_RGBA8888);
        setImageState(LOADING_FINISHED);
        return;
    }

    if (m_state == LOADING)
        return;

    setImageState(LOADING);
    QNetworkRequest request(m_source);
    m_networkAccessManager->get(request);
}

/*!
 * \qmlproperty string TextureImage::errorString()
 * Contains the error string if an error happened during image creation.
 */
/*!
 * \internal
 */
QString CanvasTextureImage::errorString() const
{
    return m_errorString;
}

/*!
 * \internal
 */
void CanvasTextureImage::handleReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        setImageState(LOADING_ERROR);
        m_errorString = reply->errorString();
        emit errorStringChanged(m_errorString);
        return;
    }

    m_image.loadFromData(reply->readAll());

    setImageState(LOADING_FINISHED);
}

/*!
 * \internal
 */
QImage & CanvasTextureImage::getImage()
{
    return m_image;
}

/*!
 * \internal
 */
QVariant *CanvasTextureImage::anything()
{
    return m_anyValue;
}

/*!
 * \internal
 */
void CanvasTextureImage::setAnything(QVariant *value)
{
    if (m_anyValue == value)
        return;
    m_anyValue = value;
    emit anythingChanged(value);
}

/*!
 * \qmlproperty TextureImageState TextureImage::imageState()
 * Contains the texture image state. It is one of \c{TextureImage.INITIALIZED},
 * \c{TextureImage.LOAD_PENDING}, \c{TextureImage.LOADING}, \c{TextureImage.LOADING_FINISHED} or
 * \c{TextureImage.LOADING_ERROR}.
 */
/*!
 * \internal
 */
CanvasTextureImage::TextureImageState CanvasTextureImage::imageState()
{
    return m_state;
}

/*!
 * \internal
 */
void CanvasTextureImage::setImageState(TextureImageState state)
{
    if (state == m_state)
        return;
    m_state = state;
    emit imageStateChanged(state);
}

/*!
 * \qmlproperty int TextureImage::width()
 * Contains the texture image width.
 */
/*!
 * \internal
 */
int CanvasTextureImage::width()
{
    if (m_state != LOADING_FINISHED)
        return 0;

    return m_image.width();
}

/*!
 * \qmlproperty int TextureImage::height()
 * Contains the texture image height.
 */
/*!
 * \internal
 */
int CanvasTextureImage::height()
{
    if (m_state != LOADING_FINISHED)
        return 0;

    return m_image.height();
}

/*!
 * \internal
 */
void CanvasTextureImage::emitImageLoadedSGRT()
{
}

/*!
 * \internal
 */
void CanvasTextureImage::emitImageLoadingErrorSGRT()
{
}


/*!
 * \internal
 */
uchar *CanvasTextureImage::convertToFormat(CanvasContext::glEnums format, bool flipY)
{
    if (m_pixelCacheFormat == format && m_pixelCacheFlipY == flipY)
        return m_pixelCache;

    // Destroy the pixel cache
    delete m_pixelCache;
    m_pixelCache = 0;
    m_pixelCacheFormat = CanvasContext::NONE;

    // Flip the image if needed
    if (m_pixelCacheFlipY != flipY) {
        m_image = m_image.mirrored(false, true);
        m_pixelCacheFlipY = flipY;
    }
    m_glImage = m_image.convertToFormat(QImage::Format_RGBA8888);

    // Get latest data for the conversion
    uchar *origPixels = m_glImage.bits();
    int width = m_glImage.width();
    int height = m_glImage.height();

    // Handle format conversions if needed
    switch (format) {
    case CanvasContext::UNSIGNED_BYTE: {
        return origPixels;
        break;
    }
    case CanvasContext::UNSIGNED_SHORT_5_6_5: {
        ushort *pixels = new ushort[width * height];
        ushort pixel;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int srcIdx = y * width * 4 + x * 4;
                pixel =  ((origPixels[srcIdx++] >> 3) & 0x1F) << 11;
                pixel |= ((origPixels[srcIdx++] >> 2) & 0x3F) << 5;
                pixel |= ((origPixels[srcIdx++] >> 3) & 0x1F) << 0;
                pixels[y * width + x] = pixel;
            }
        }
        m_pixelCacheFormat = CanvasContext::UNSIGNED_SHORT_5_6_5;
        m_pixelCache = (uchar *)pixels;
        return m_pixelCache;
    }
    case CanvasContext::UNSIGNED_SHORT_4_4_4_4: {
        ushort *pixels = new ushort[width * height];
        ushort pixel;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int srcIdx = y * width * 4 + x * 4;
                pixel =  ((origPixels[srcIdx++] >> 4) & 0x0F) << 12;
                pixel |= ((origPixels[srcIdx++] >> 4) & 0x0F) << 8;
                pixel |= ((origPixels[srcIdx++] >> 4) & 0x0F) << 4;
                pixel |= ((origPixels[srcIdx++] >> 4) & 0x0F) << 0;
                pixels[y * width + x] = pixel;
            }
        }
        m_pixelCacheFormat = CanvasContext::UNSIGNED_SHORT_4_4_4_4;
        m_pixelCache = (uchar *)pixels;
        return m_pixelCache;
    }
    case CanvasContext::UNSIGNED_SHORT_5_5_5_1: {
        ushort *pixels = new ushort[width * height];
        ushort pixel;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int srcIdx = y * width * 4 + x * 4;
                pixel =  ((origPixels[srcIdx++] >> 3) & 0x1F) << 11;
                pixel |= ((origPixels[srcIdx++] >> 3) & 0x1F) << 6;
                pixel |= ((origPixels[srcIdx++] >> 3) & 0x1F) << 1;
                pixel |= ((origPixels[srcIdx++] >> 7) & 0x01) << 0;
                pixels[y * width + x] = pixel;
            }
        }
        m_pixelCacheFormat = CanvasContext::UNSIGNED_SHORT_5_5_5_1;
        m_pixelCache = (uchar *)pixels;
        return m_pixelCache;
    }
    default: {
        qDebug() << "TexImage3D::" << __FUNCTION__ << ":INVALID_ENUM Invalid type enum";
        break;
    }
    }

    return 0;
}

/*!
 * \internal
 */
QDebug operator<<(QDebug dbg, const CanvasTextureImage *texImage)
{
    if (texImage)
        dbg.nospace() << "TexImage3D("<< ((void*) texImage) << texImage->m_name << ")";
    else
        dbg.nospace() << "TexImage3D("<< ((void*) texImage) <<")";
    return dbg.maybeSpace();
}

