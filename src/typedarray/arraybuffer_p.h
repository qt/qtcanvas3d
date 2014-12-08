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

#ifndef ARRAYBUFFER_P_H
#define ARRAYBUFFER_P_H

#include "../canvas3dcommon_p.h"
#include <QObject>

QT_CANVAS3D_BEGIN_NAMESPACE

class CanvasArrayBuffer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CanvasArrayBuffer)
    Q_PROPERTY(unsigned long byteLength READ byteLength NOTIFY byteLengthChanged)

public:
    explicit CanvasArrayBuffer(QObject *parent = 0);
    explicit CanvasArrayBuffer(unsigned long length, QObject *parent = 0);
    ~CanvasArrayBuffer();

    unsigned long byteLength() const;
    Q_INVOKABLE CanvasArrayBuffer *slice(long begin);
    Q_INVOKABLE CanvasArrayBuffer *slice(long begin, long end);
    Q_INVOKABLE bool isView(const QObject &value);

    void copyRawDataFrom(unsigned char *rawData, unsigned long byteLength);

signals:
    void byteLengthChanged(unsigned long byteLength);

public slots:

private:
    unsigned char *rawData();

    unsigned long m_length;
    unsigned char *m_rawData;

    // Only the type based views have access to the modifiable rawData
    friend class CanvasInt8Array;
    friend class CanvasUint8Array;
    friend class CanvasInt16Array;
    friend class CanvasUint16Array;
    friend class CanvasInt32Array;
    friend class CanvasUint32Array;
    friend class CanvasFloat32Array;
    friend class CanvasFloat64Array;
    friend class CanvasUint8ClampedArray;
    friend class CanvasArrayBufferView;
    friend class CanvasTypedArray;
    friend class CanvasContext;
};

QT_CANVAS3D_END_NAMESPACE

#endif // ARRAYBUFFER_P_H
