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

#ifndef TEXIMAGE3D_P_H
#define TEXIMAGE3D_P_H

#include "context3d_p.h"
#include "abstractobject3d_p.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QImage>
#include <QNetworkReply>

class CanvasTextureImage : public CanvasAbstractObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CanvasTextureImage)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(TextureImageState imageState READ imageState NOTIFY imageStateChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_ENUMS(TextureImageState)

public:
    enum TextureImageState {
        INITIALIZED = 0,
        LOAD_PENDING,
        LOADING,
        LOADING_FINISHED,
        LOADING_ERROR
    };

    explicit CanvasTextureImage(QObject *parent = 0);
    virtual ~CanvasTextureImage();

    Q_INVOKABLE ulong id();

    QVariant *anything();
    void setAnything(QVariant *value);

    const QUrl &source() const;
    void setSource(const QUrl &src);
    TextureImageState imageState();
    int width();
    int height();
    QString errorString() const;

    void load();
    void handleReply(QNetworkReply *reply);
    QImage &getImage();
    uchar *getImageData();
    void *convertToFormat(CanvasContext::glEnums format);

    void emitImageLoadedSGRT();
    void emitImageLoadingErrorSGRT();

    friend QDebug operator<< (QDebug d, const CanvasTextureImage *buffer);

private:
    void setImageState(TextureImageState state);

signals:
    void sourceChanged(QUrl source);
    void imageStateChanged(TextureImageState state);
    void widthChanged(int width);
    void heightChanged(int height);
    void errorStringChanged(const QString errorString);
    void anythingChanged(QVariant *value);

private:
    QNetworkAccessManager *m_networkAccessManager;
    int m_requestId;
    QImage m_image;
    QUrl m_source;
    TextureImageState m_state;
    QString m_errorString;
    uchar *m_pixelCache;
    CanvasContext::glEnums m_pixelCacheFormat;
    QImage m_glImage;
    QVariant *m_anyValue;
};

#endif // TEXIMAGE3D_P_H
