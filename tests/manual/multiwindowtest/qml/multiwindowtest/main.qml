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

import QtQuick 2.6
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2

Window {
    id: mainview
    width: 400
    height: 600
    visible: true
    title: "Windows:" + windowCount + " Canvases:" + canvasCount

    property var windowList: []
    property var canvasList: []
    property var windowComponent: null
    property var fboCanvasComponent: null
    property var itemTextureCanvasComponent: null
    property int windowCount: 0
    property int canvasCount: 0

    onClosing: {
       for (var i = windowCount - 1; i >= 0; i--)
           windowList[i].close()
    }

    ColumnLayout {
        id: controlLayout
        spacing: 1
        anchors.fill: parent
        visible: true

        Button {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "New window"
            onClicked: {
                if (windowComponent === null)
                    windowComponent = Qt.createComponent("canvaswindow.qml")
                var window = windowComponent.createObject(null)
                window.setManager(mainview)
                windowList[windowCount] = window
                windowCount++
                window.x = windowCount * 20
                window.y = windowCount * 20
            }
        }

        Button {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "New framebuffer canvas"
            onClicked: {
                createFboCanvas(null)
            }
        }

        Button {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "New QuickTexture  canvas"
            onClicked: {
                createItemTextureCanvas(null)
            }
        }

        Button {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Delete random canvas"
            onClicked: {
                deleteCanvas(chooseRandomCanvas())
            }
        }

        Button {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Delete first canvas"
            onClicked: {
                if (canvasCount > 0)
                    deleteCanvas(canvasList[0])
            }
        }

        Button {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Delete last canvas"
            onClicked: {
                if (canvasCount > 0)
                    deleteCanvas(canvasList[canvasCount - 1])
            }
        }
    }

    function removeWindow(win) {
        var found = false
        for (var i = 0; i < windowCount; i++) {
            if (windowList[i] === win) {
                found = true
                windowCount--
            }
            if (found) {
                if (i < windowCount)
                    windowList[i] = windowList[i + 1]
                else
                    windowList[i] = null
            }
        }
    }

    function deleteCanvas(canvas) {
        var found = false
        for (var i = 0; i < canvasCount; i++) {
            if (canvasList[i] === canvas) {
                found = true
                canvasCount--
                console.log("Canvas deleted: ", canvasList[i].canvasName)
                canvasList[i].destroy()
            }
            if (found) {
                if (i < canvasCount)
                    canvasList[i] = canvasList[i + 1]
                else
                    canvasList[i] = null
            }
        }
    }

    function chooseRandomCanvas() {
        if (canvasCount > 0) {
            var index = Math.floor((Math.random() * canvasCount))
            console.log("Random canvas selected: ", canvasList[index].canvasName)
            return canvasList[index]
        } else {
            return null
        }
    }

    function createFboCanvas(canvasArea) {
        if (fboCanvasComponent === null)
            fboCanvasComponent = Qt.createComponent("framebuffer.qml")
        var canvas = fboCanvasComponent.createObject(null)
        canvas.parent = canvasArea
        canvas.canvasName = "FBO canvas " + canvasCount
        canvasList[canvasCount] = canvas
        canvasCount++
        console.log("Created ", canvas.canvasName)
        return canvas
    }

    function createItemTextureCanvas(canvasArea) {
        if (itemTextureCanvasComponent === null)
            itemTextureCanvasComponent = Qt.createComponent("quickitemtexture.qml")
        var canvas = itemTextureCanvasComponent.createObject(null)
        canvas.parent = canvasArea
        canvas.canvasName = "QuickItem canvas " + canvasCount
        canvasList[canvasCount] = canvas
        canvasCount++
        console.log("Created ", canvas.canvasName)
        return canvas
    }
}
