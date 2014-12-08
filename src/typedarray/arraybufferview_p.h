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

#ifndef ARRAYBUFFERVIEW_P_H
#define ARRAYBUFFERVIEW_P_H

#include "../context3d_p.h"
#include "../abstractobject3d_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

class CanvasArrayBuffer;

class CanvasArrayBufferView : public CanvasAbstractObject
{
    Q_OBJECT
    Q_PROPERTY(CanvasArrayBuffer* buffer READ buffer NOTIFY bufferChanged)
    Q_PROPERTY(unsigned long byteOffset READ byteOffset NOTIFY byteOffsetChanged)
    Q_PROPERTY(unsigned long byteLength READ byteLength NOTIFY byteLengthChanged)

public:
    enum canvasArrayBufferTypes {
        ARRAY_BUFFER_UNKNOWN = 0,
        ARRAY_BUFFER_UINT_8,
        ARRAY_BUFFER_UINT_8_CLAMPED,
        ARRAY_BUFFER_UINT_16,
        ARRAY_BUFFER_UINT_32,
        ARRAY_BUFFER_INT_8,
        ARRAY_BUFFER_INT_16,
        ARRAY_BUFFER_INT_32,
        ARRAY_BUFFER_FLOAT_32,
        ARRAY_BUFFER_FLOAT_64
    };

    explicit CanvasArrayBufferView(CanvasArrayBuffer *buffer, unsigned long byteOffset,
                                   QObject *parent = 0);
    virtual ~CanvasArrayBufferView();

    unsigned char* rawDataCptr() const;

    CanvasArrayBuffer *buffer();
    unsigned long byteOffset();
    unsigned long byteLength();
    virtual canvasArrayBufferTypes bufferType() = 0;

signals:
    void bufferChanged(CanvasArrayBuffer *buffer);
    void byteOffsetChanged(unsigned long byteLength);
    void byteLengthChanged(unsigned long byteLength);

protected:
    CanvasArrayBuffer *m_buffer;
    unsigned long m_byteOffset;
};

QT_CANVAS3D_END_NAMESPACE

#endif // ARRAYBUFFERVIEW_P_H
