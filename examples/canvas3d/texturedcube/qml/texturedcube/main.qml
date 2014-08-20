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

import QtQuick 2.0
import QtCanvas3D 1.0

//! [4]
import "texturedcube.js" as GLCode
//! [4]

Item {
    id: mainview
    width: 1280
    height: 768
    visible: true

    //! [0]
    Canvas3D {
        id: canvas3d
        anchors.fill: parent
        //! [0]
        //! [2]
        imageLoader: textureImageLoader
        //! [2]
        focus: true
        property double xRotAnim: 0
        property double yRotAnim: 0
        property double zRotAnim: 0
        property bool isRunning: true

        //! [1]
        // Called by Canvas 3D once on Scene Graph render thread
        // to allow one time initializations to happen
        function initGL() {
            GLCode.initGL(canvas3d, textureImageLoader);
        }

        // Called by Canvas 3D for every frame
        // rendered on Scene Graph render thread
        function renderGL() {
            GLCode.renderGL(canvas3d);
        }
        //! [1]

        //! [5]
        SequentialAnimation {
            id: objAnimationX
            loops: Animation.Infinite
            running: true
            NumberAnimation {
                target: canvas3d
                property: "xRotAnim"
                from: 0.0
                to: 120.0
                duration: 7000
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: canvas3d
                property: "xRotAnim"
                from: 120.0
                to: 0.0
                duration: 7000
                easing.type: Easing.InOutQuad
            }
        }
        //! [5]

        SequentialAnimation {
            id: objAnimationY
            loops: Animation.Infinite
            running: true
            NumberAnimation {
                target: canvas3d
                property: "yRotAnim"
                from: 0.0
                to: 240.0
                duration: 5000
                easing.type: Easing.InOutCubic
            }
            NumberAnimation {
                target: canvas3d
                property: "yRotAnim"
                from: 240.0
                to: 0.0
                duration: 5000
                easing.type: Easing.InOutCubic
            }
        }

        SequentialAnimation {
            id: objAnimationZ
            loops: Animation.Infinite
            running: true
            NumberAnimation {
                target: canvas3d
                property: "zRotAnim"
                from: -100.0
                to: 100.0
                duration: 3000
                easing.type: Easing.InOutSine
            }
            NumberAnimation {
                target: canvas3d
                property: "zRotAnim"
                from: 100.0
                to: -100.0
                duration: 3000
                easing.type: Easing.InOutSine
            }
        }
    }

    //! [3]
    TextureImageLoader {
        id: textureImageLoader

        function imageLoaded(textureImage) {
            if (canvas3d.logAllCalls)
                console.log("Texture loaded, size "+textureImage.width+"x"+textureImage.height);
            GLCode.textureLoaded(textureImage);
        }

        function imageLoadingError(textureImage) {
            if (GLCode.textureLoadError !== undefined) {
                GLCode.textureLoadError(textureImage);
            }
            console.log("Texture load FAILED, "+textureImage.errorString);
        }
    }
    //! [3]
}
