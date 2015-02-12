/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcanvas3d_plugin.h"

#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

QT_CANVAS3D_BEGIN_NAMESPACE

void QtCanvas3DPlugin::registerTypes(const char *uri)
{
    // @uri com.digia.qtcanvas3d

    // QTCANVAS3D CORE API
    qmlRegisterSingletonType<CanvasTextureImageFactory>(uri,
                                                        1, 0,
                                                        "TextureImageFactory",
                                                        CanvasTextureImageFactory::texture_image_factory_provider);

    qmlRegisterUncreatableType<CanvasTextureImage>(uri,
                                        1, 0,
                                        "TextureImage",
                                        QLatin1String("Trying to create uncreatable: TextureImage, use TextureImageFactory.newTexImage() instead."));
    qmlRegisterType<Canvas>(uri,
                            1, 0,
                            "Canvas3D");
    qmlRegisterType<CanvasContextAttributes>(uri,
                                             1, 0,
                                             "ContextAttributes3D");
    qmlRegisterUncreatableType<CanvasShaderPrecisionFormat>(uri,
                                                            1, 0,
                                                            "ShaderPrecisionFormat3D",
                                                            QLatin1String("Trying to create uncreatable: ShaderPrecisionFormat3D."));
    qmlRegisterUncreatableType<CanvasContext>(uri,
                                              1, 0,
                                              "Context3D",
                                              QLatin1String("Trying to create uncreatable: Context3D, use Canvas3D.getContext() instead."));
    qmlRegisterUncreatableType<CanvasActiveInfo>(uri,
                                                 1, 0,
                                                 "ActiveInfo3D",
                                                 QLatin1String("Trying to create uncreatable: ActiveInfo3D, use Context3D.getActiveAttrib() or Context3D.getActiveUniform() instead."));
    qmlRegisterUncreatableType<CanvasTexture>(uri,
                                              1, 0,
                                              "Texture3D",
                                              QLatin1String("Trying to create uncreatable: Texture3D, use Context3D.createTexture() instead."));
    qmlRegisterUncreatableType<CanvasShader>(uri,
                                             1, 0,
                                             "Shader3D",
                                             QLatin1String("Trying to create uncreatable: Shader3D, use Context3D.createShader() instead."));
    qmlRegisterUncreatableType<CanvasFrameBuffer>(uri,
                                                  1, 0,
                                                  "FrameBuffer3D",
                                                  QLatin1String("Trying to create uncreatable: FrameBuffer3D, use Context3D.createFramebuffer() instead."));
    qmlRegisterUncreatableType<CanvasRenderBuffer>(uri,
                                                   1, 0,
                                                   "RenderBuffer3D",
                                                   QLatin1String("Trying to create uncreatable: RenderBuffer3D, use Context3D.createRenderbuffer() instead."));
    qmlRegisterUncreatableType<CanvasProgram>(uri,
                                              1, 0,
                                              "Program3D",
                                              QLatin1String("Trying to create uncreatable: Program3D, use Context3D.createProgram() instead."));
    qmlRegisterUncreatableType<CanvasBuffer>(uri,
                                             1, 0,
                                             "Buffer3D",
                                             QLatin1String("Trying to create uncreatable: Buffer3D, use Context3D.createBuffer() instead."));
    qmlRegisterUncreatableType<CanvasUniformLocation>(uri,
                                                      1, 0,
                                                      "UniformLocation3D",
                                                      QLatin1String("Trying to create uncreatable: UniformLocation3D, use Context3D.getUniformLocation() instead."));

    // EXTENSIONS
    qmlRegisterUncreatableType<CanvasGLStateDump>(uri,
                                                  1, 0,
                                                  "GLStateDumpExt",
                                                  QLatin1String("Trying to create uncreatable: GLStateDumpExt, use Context3D.getExtension(\"" QT_CANVAS3D_GL_STATE_DUMP_EXT_NAME "\") instead."));
}

QT_CANVAS3D_END_NAMESPACE

QT_END_NAMESPACE
