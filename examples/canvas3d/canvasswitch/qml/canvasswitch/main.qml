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

import QtQuick 2.2
import QtCanvas3D 1.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

//! [4]
import "triangle.js" as GLCodeTriangle
import "cube.js" as GLCodeCube
//! [4]

Item {
    id: mainview
    width: 1280
    height: 768
    visible: true

    property var activeObject: GLCodeTriangle
    property var canvas3d
    property double xRotAnim: 0
    property double yRotAnim: 0
    property double zRotAnim: 0

    //! [0]
    Component.onCompleted: {
        createCanvas()
    }
    //! [0]

    SequentialAnimation {
        id: objAnimationX
        loops: Animation.Infinite
        running: true
        NumberAnimation {
            target: mainview
            property: "xRotAnim"
            from: 0.0
            to: 120.0
            duration: 7000
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: mainview
            property: "xRotAnim"
            from: 120.0
            to: 0.0
            duration: 7000
            easing.type: Easing.InOutQuad
        }
    }

    SequentialAnimation {
        id: objAnimationY
        loops: Animation.Infinite
        running: true
        NumberAnimation {
            target: mainview
            property: "yRotAnim"
            from: 0.0
            to: 240.0
            duration: 5000
            easing.type: Easing.InOutCubic
        }
        NumberAnimation {
            target: mainview
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
            target: mainview
            property: "zRotAnim"
            from: -100.0
            to: 100.0
            duration: 3000
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            target: mainview
            property: "zRotAnim"
            from: 100.0
            to: -100.0
            duration: 3000
            easing.type: Easing.InOutSine
        }
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        RowLayout {
            id: buttonrow
            anchors.margins: 10
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter

            //! [5]
            Button {
                id: toggleButton
                text: "Toggle Canvas"
                property bool firstObject: true
                //! [5]
                //! [6]
                onClicked: {
                    canvas3d.destroy()
                    firstObject = !firstObject
                    if (firstObject)
                        activeObject = GLCodeTriangle
                    else
                        activeObject = GLCodeCube
                    createCanvas()
                }
                //! [6]
            }
            Button {
                id: quitButton
                text: "Quit"
                onClicked: {
                    canvas3d.destroy()
                    Qt.quit()
                }
            }
        }
    }

    //! [1]
    function createCanvas() {
        //! [2]
        // Create canvas dynamically to allow changing js objects on the fly
        canvas3d = Qt.createQmlObject("
        import QtQuick 2.2
        import QtCanvas3D 1.0
        Canvas3D {
            property double xRotAnim: mainview.xRotAnim
            property double yRotAnim: mainview.yRotAnim
            property double zRotAnim: mainview.zRotAnim
            onInitGL: activeObject.initGL(canvas3d)
            onRenderGL: activeObject.renderGL(canvas3d)
        }", layout)
        //! [2]
        //! [3]
        canvas3d.anchors.margins = 10
        canvas3d.anchors.top = layout.top
        canvas3d.anchors.left = layout.left
        canvas3d.anchors.right = layout.right
        canvas3d.anchors.bottom = buttonrow.top
        //! [3]
    }
    //! [1]
}
