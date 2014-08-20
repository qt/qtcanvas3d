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

#include "teximage3dloader_p.h"
#include "canvas3d_p.h"
#include "canvas3dcommon_p.h"

/*!
 * \qmltype TextureImageLoader
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Texture image loader.
 *
 * Texture image loader that allows loading of 2D images to be used as input of Context3D
 * texture upload calls.
 *
 * \sa Context3D, TextureImage, {QML Canvas 3D QML Types}
 */

/*!
 * \internal
 */
CanvasTextureImageLoader::CanvasTextureImageLoader(QObject *parent) :
    QObject(parent),
    m_logAllCalls(false),
    m_logAllErrors(true)
{
}

/*!
 * \internal
 */
CanvasTextureImageLoader::~CanvasTextureImageLoader()
{
}

/*!
 * \internal
 */
void CanvasTextureImageLoader::setLogAllCalls(bool logCalls)
{
    if (m_logAllCalls != logCalls) {
        m_logAllCalls = logCalls;
        emit logAllCallsChanged(logCalls);
    }
}

/*!
 * \qmlproperty bool TextureImageLoader::logAllCalls
 * Specifies if all TextureImageLoader method calls (including internal ones) are logged to the
 * console.
 * Defaults to \c{false}.
 */
/*!
 * \internal
 */
bool CanvasTextureImageLoader::logAllCalls() const
{
    return m_logAllCalls;
}

/*!
 * \internal
 */
void CanvasTextureImageLoader::setLogAllErrors(bool logErrors)
{
    if (m_logAllErrors != logErrors) {
        m_logAllErrors = logErrors;
        emit logAllErrorsChanged(logErrors);
    }
}

/*!
 * \qmlproperty bool TextureImageLoader::logAllErrors
 * Specifies if all TextureImageLoader errors are logged to the console. Defaults to \c{true}.
 */
/*!
 * \internal
 */
bool CanvasTextureImageLoader::logAllErrors() const
{
    return m_logAllErrors;
}

/*!
 * \internal
 */
void CanvasTextureImageLoader::emitImageLoaded(CanvasTextureImage *textureImage)
{
    // Can't use signals as those change the signal processing to main thread
    // Using invokeMethod() preserves execution on scene graph render thread.
    if (m_logAllCalls) qDebug() << "TexImage3DLoader::" << __FUNCTION__;
    QVariant ignoredReturnedValue;
    QMetaObject::invokeMethod(this, "imageLoaded",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(QVariant, ignoredReturnedValue),
                              Q_ARG(QVariant, QVariant::fromValue(textureImage)));
}

/*!
 * \internal
 */
void CanvasTextureImageLoader::emitImageLoadingError(CanvasTextureImage *textureImage)
{
    // Can't use signals as those change the signal processing to main thread
    // Using invokeMethod() preserves execution on scene graph render thread.
    if (m_logAllCalls) qDebug() << "TexImage3DLoader::" << __FUNCTION__;
    QVariant ignoredReturnedValue;
    QMetaObject::invokeMethod(this, "imageLoadingError",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(QVariant, ignoredReturnedValue),
                              Q_ARG(QVariant, QVariant::fromValue(textureImage)));
}

/*!
 * \qmlmethod TextureImage TextureImageLoader::loadImage(url url)
 * Loads an image located in \a url and creates a TextureImage of it.
 */
/*!
 * \internal
 */
CanvasTextureImage *CanvasTextureImageLoader::loadImage(const QUrl &url)
{
    if (m_logAllCalls) qDebug() << "TexImage3DLoader::" << __FUNCTION__ << "(url:" << url << ")";

    if (!m_canvas && m_logAllErrors) qDebug() << "TexImage3DLoader::" << __FUNCTION__ << ": ERROR tried to load image before setting as property of a canvas.";

    CanvasContext *context = m_canvas->context();
    if (!context) {
        if (m_logAllErrors) qDebug() << "TexImage3DLoader::" << __FUNCTION__ << ": ERROR tried to load image before GL context was created.";
        return 0;
    }

    CanvasTextureImage *img;
    if (m_urlToImageMap.contains(url) && m_urlToImageMap.values(url).size() != 0) {
        img = m_urlToImageMap[url];
    } else {
        img = new CanvasTextureImage(context);
        m_urlToImageMap[url] = img;
        m_loadingImagesList << img;
        img->setSource(url);
    }

    return img;
}

/*!
 * \internal
 */
void CanvasTextureImageLoader::setCanvas(Canvas *canvas)
{
    m_canvas = canvas;
}

/*!
 * \internal
 */
void CanvasTextureImageLoader::notifyLoadedImages()
{
    if (!m_loadingImagesList.size())
        return;

    if (m_logAllCalls) qDebug() << "TexImage3DLoader::" << __FUNCTION__ << "(m_loadingImagesList.size():"<<m_loadingImagesList.size()<<")";

    QMutableListIterator<CanvasTextureImage *> it(m_loadingImagesList);
    while (it.hasNext()) {
        CanvasTextureImage *loader = it.next();
        if (loader->imageState() == CanvasTextureImage::LOADING_FINISHED) {
            m_loadingImagesList.removeOne(loader);
            emitImageLoaded(loader);
        } else if (loader->imageState() == CanvasTextureImage::LOADING_ERROR) {
            m_loadingImagesList.removeOne(loader);
            emitImageLoadingError(loader);
        }
    }
}
