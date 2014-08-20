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
//! [0]
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
//! [0]

import "interaction.js" as GLCode

Item {
    id: mainview
    width: 1280
    height: 768
    visible: true

    Canvas3D {
        id: canvas3d
        anchors.fill: parent
        imageLoader: textureImageLoader
        focus: true
        //! [3]
        property double xRotSlider: 0
        property double yRotSlider: 0
        property double zRotSlider: 0
        //! [3]

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
    }

    //! [1]
    RowLayout {
        id: controlLayout
        spacing: 5
        y: parent.height - 100
        width: parent.width
        height: 100
        visible: true
        //! [1]

        Label {
            id: xRotLabel
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: false
            text: "X-axis:"
            color: "#FFFFFF"
        }

        //! [2]
        Slider {
            id: xSlider
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth : 300
            Layout.fillWidth: true
            minimumValue: 0;
            maximumValue: 360;
            //! [4]
            onValueChanged: canvas3d.xRotSlider = value;
            //! [4]
        }
        //! [2]

        Label {
            id: yRotLabel
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: false
            text: "Y-axis:"
            color: "#FFFFFF"
        }

        Slider {
            id: ySlider
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth : 300
            Layout.fillWidth: true
            minimumValue: 0;
            maximumValue: 360;
            onValueChanged: canvas3d.yRotSlider = value;
        }

        Label {
            id: zRotLabel
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: false
            text: "Z-axis:"
            color: "#FFFFFF"
        }

        Slider {
            id: zSlider
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth : 300
            Layout.fillWidth: true
            minimumValue: 0;
            maximumValue: 360;
            onValueChanged: canvas3d.zRotSlider = value;
        }
    }

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
}
