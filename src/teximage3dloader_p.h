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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtCanvas3D API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef TEXIMAGE3DLOADER_P_H
#define TEXIMAGE3DLOADER_P_H

#include "abstractobject3d_p.h"
#include "teximage3d_p.h"

class Canvas;

class CanvasTextureImageLoader : public CanvasAbstractObject
{
    Q_OBJECT
    Q_PROPERTY(CanvasTextureImage* image READ image)
    Q_PROPERTY(bool logAllCalls READ logAllCalls WRITE setLogAllCalls NOTIFY logAllCallsChanged)
    Q_PROPERTY(bool logAllErrors READ logAllErrors WRITE setLogAllErrors NOTIFY logAllErrorsChanged)

public:
    explicit CanvasTextureImageLoader(QObject *parent = 0);
    virtual ~CanvasTextureImageLoader();

    Q_INVOKABLE CanvasTextureImage *loadImage(const QUrl &url);

    void setLogAllCalls(bool logCalls);
    bool logAllCalls() const;
    void setLogAllErrors(bool logErrors);
    bool logAllErrors() const;
    void setCanvas(Canvas *canvas);
    void notifyLoadedImages();
    void emitImageLoaded(CanvasTextureImage *textureImage);
    void emitImageLoadingError(CanvasTextureImage *textureImage);
    inline CanvasTextureImage *image() { return m_image; }

signals:
    void imageLoaded();
    void imageLoadingFailed();
    void logAllCallsChanged(bool logCalls);
    void logAllErrorsChanged(bool logErrors);

private:
    bool m_logAllCalls;
    bool m_logAllErrors;
    QMap<QUrl, CanvasTextureImage *> m_urlToImageMap;
    QList<CanvasTextureImage *> m_loadingImagesList;
    CanvasTextureImage *m_image;
    Canvas *m_canvas;
};

#endif // TEXIMAGE3DLOADER_P_H
