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

import QtQuick 2.0
import QtCanvas3D 1.0

import "imagecube.js" as GLCode

Canvas3D {
    id: cube
    //! [0]
    property color backgroundColor: "#FCFCFC"
    property real angleOffset: -180 / 8.0
    property real xRotation: 0
    property real yRotation: 0
    property real zRotation: 0
    property string page1: ""
    //! [0]
    property string page2: ""
    property string page3: ""
    property string page4: ""
    property string page5: ""
    property string page6: ""
    state: "page6"

    onBackgroundColorChanged: { GLCode.setBackgroundColor(cube.backgroundColor); }

    //! [1]
    states: [
        State {
            name: "page1"
            PropertyChanges { target: cube; xRotation: 0; }
            PropertyChanges { target: cube; yRotation: 180 * 1.5 + angleOffset; }
            PropertyChanges { target: cube; zRotation: 0 }
        },
        //! [1]
        State {
            name: "page2"
            PropertyChanges { target: cube; xRotation: 0; }
            PropertyChanges { target: cube; yRotation: 180 * 1.0 + angleOffset; }
            PropertyChanges { target: cube; zRotation: 0 }
        },
        State {
            name: "page3"
            PropertyChanges { target: cube; xRotation: 0; }
            PropertyChanges { target: cube; yRotation: 180 * 0.5 + angleOffset; }
            PropertyChanges { target: cube; zRotation: 0 }
        },
        State {
            name: "page4"
            PropertyChanges { target: cube; xRotation: 0; }
            PropertyChanges { target: cube; yRotation: 0 + angleOffset; }
            PropertyChanges { target: cube; zRotation: 0 }
        },
        State {
            name: "page5"
            PropertyChanges { target: cube; xRotation: 180 / 2.0; }
            PropertyChanges { target: cube; yRotation: 0; }
            PropertyChanges { target: cube; zRotation: -angleOffset; }
        },
        State {
            name: "page6"
            PropertyChanges { target: cube; xRotation: -180 / 2.0; }
            PropertyChanges { target: cube; yRotation: 0; }
            PropertyChanges { target: cube; zRotation: angleOffset; }
        }
    ]

    //! [2]
    transitions: [
        Transition {
            id: turnTransition
            from: "*"
            to: "*"
            RotationAnimation {
                properties: "xRotation,yRotation,zRotation"
                easing.type: Easing.InOutCubic
                direction: RotationAnimation.Shortest
                duration: 450
            }
        }
    ]
    //! [2]

    //! [3]
    onInitializeGL: {
        GLCode.initializeGL(cube);
    }

    onPaintGL: {
        GLCode.paintGL(cube);
    }

    onResizeGL: {
        GLCode.onResizeGL(cube);
    }
    //! [3]

    SwipeArea {
        id: swipeArea
        anchors.fill: parent

        onSwipeRight: {
            if (cube.state === "page1")
                cube.state = "page4";
            else if (cube.state == "page2")
                cube.state = "page1";
            else if (cube.state == "page3")
                cube.state = "page2";
            else if (cube.state == "page4")
                cube.state = "page3";
            else if (cube.state == "page5")
                cube.state = "page3";
            else if (cube.state == "page6")
                cube.state = "page3";
        }

        onSwipeLeft: {
            if (cube.state === "page1")
                cube.state = "page2";
            else if (cube.state == "page2")
                cube.state = "page3";
            else if (cube.state == "page3")
                cube.state = "page4";
            else if (cube.state == "page4")
                cube.state = "page1";
            else if (cube.state == "page5")
                cube.state = "page1";
            else if (cube.state == "page6")
                cube.state = "page1";
        }

        onSwipeUp: {
            if (cube.state === "page1")
                cube.state = "page6";
            else if (cube.state == "page2")
                cube.state = "page6";
            else if (cube.state == "page3")
                cube.state = "page6";
            else if (cube.state == "page4")
                cube.state = "page6";
            else if (cube.state == "page5")
                cube.state = "page4";
            else if (cube.state == "page6")
                cube.state = "page2";
        }

        onSwipeDown: {
            if (cube.state === "page1")
                cube.state = "page5";
            else if (cube.state == "page2")
                cube.state = "page5";
            else if (cube.state == "page3")
                cube.state = "page5";
            else if (cube.state == "page4")
                cube.state = "page5";
            else if (cube.state == "page5")
                cube.state = "page2";
            else if (cube.state == "page6")
                cube.state = "page4";
        }
    }

}

