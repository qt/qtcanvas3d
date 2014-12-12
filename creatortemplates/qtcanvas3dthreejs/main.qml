import QtQuick 2.0
import QtCanvas3D 1.0

import "materials.js" as GLCode

Item {
    id: mainview
    width: 1200
    height: 700
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

