import QtQuick 2.0
import QtCanvas3D 1.0

import "cube.js" as GLCode

Item {
    id: mainview
    width: 1280
    height: 768
    visible: true

    Canvas3D {
        id: canvas3d
        anchors.fill: parent
        focus: true

        onInitGL: {
            GLCode.initGL(canvas3d);
        }

        onRenderGL: {
            GLCode.renderGL(canvas3d);
        }
    }
}

