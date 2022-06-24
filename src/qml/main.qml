// clang-format off

import QtQuick 2.0
import QtQuick.Window 2.0
import QtLocation 5.6
import QtPositioning 5.6
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Map
{
    property variant cttc: QtPositioning.coordinate(41.27504, 1.987709)
    property variant scaleLengths: [5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000, 2000000]

    function formatDistance(distance)
    {
        if (distance < 1000)
            return distance.toFixed(0) + " m";

        var km = distance/1000;
        if (km < 10)
            return km.toFixed(1) + " km";

        return km.toFixed(0) + " km";
    }
    function calculateScale()
    {
        var coord1, coord2, dist, text, f
        f = 0
        coord1 = map.toCoordinate(Qt.point(0,scale.y))
        coord2 = map.toCoordinate(Qt.point(0+scaleImage.sourceSize.width,scale.y))
        dist = Math.round(coord1.distanceTo(coord2))

        if (dist === 0) {
            // not visible
        } else {
            for (var i = 0; i < scaleLengths.length-1; i++) {
                if (dist < (scaleLengths[i] + scaleLengths[i+1]) / 2 ) {
                    f = scaleLengths[i] / dist
                    dist = scaleLengths[i]
                    break;
                }
            }
            if (f === 0) {
                f = dist / scaleLengths[i]
                dist = scaleLengths[i]
            }
        }

        text = formatDistance(dist)
        scaleImage.width = (scaleImage.sourceSize.width * f) - 2 * scaleImageLeft.sourceSize.width
        scaleText.text = text
    }

    id: map
    height: 300

    plugin: Plugin
    {
        id: mapPlugin
        name: "osm"

//        PluginParameter
//       {
//            name: "esri.mapping.maximumZoomLevel"
//            value: 19.90
//        }
    }
    center: cttc
    zoomLevel: 15

    MapPolyline // Path of the vehicle.
    {
        line.width: 3
        line.color: "red"
        opacity: 0.3
        path: m_monitor_pvt_wrapper.path
        visible: show_path.checked
    }

    MapQuickItem // Current position of the vehicle.
    {
        id: vehicle
        coordinate: m_monitor_pvt_wrapper.position

        anchorPoint.x: icon.width/2
        anchorPoint.y: icon.height/2

        sourceItem: Rectangle
        {
            id: icon
            width: 10
            height: 10
            radius: width/2
            color: "red"
        }

    }

    ColumnLayout
    {
        CheckBox // Show Path CheckBox.
        {
            id: show_path
            checked: true
            text: "Show Path"
        }

        CheckBox // Follow CheckBox.
        {
            id: follow
            checked: true
            text: "Follow"
        }
    }

    Connections
    {
        target: vehicle
        onCoordinateChanged:
        {
            if (follow.checked)
            {
                map.center = m_monitor_pvt_wrapper.position;
            }
        }
    }

    onCenterChanged:{
        scaleTimer.restart()
    }

    onZoomLevelChanged:{
        scaleTimer.restart()
    }

    onWidthChanged:{
        scaleTimer.restart()
    }

    onHeightChanged:{
        scaleTimer.restart()
    }
    Timer {
        id: scaleTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: {
            map.calculateScale()
        }
    }

    Item {
        id: scale
        visible: scaleText.text != "0 m"
        z: map.z + 3
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 20
        height: scaleText.height * 2
        width: scaleImage.width

        Image {
            id: scaleImageLeft
            source: "../images/scale_end.png"
            anchors.bottom: parent.bottom
            anchors.right: scaleImage.left
        }
        Image {
            id: scaleImage
            source: "../images/scale.png"
            anchors.bottom: parent.bottom
            anchors.right: scaleImageRight.left
        }
        Image {
            id: scaleImageRight
            source: "../images/scale_end.png"
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }
        Label {
            id: scaleText
            color: "#004EAE"
            anchors.centerIn: parent
            text: "0 m"
        }
        Component.onCompleted: {
            map.calculateScale();
        }
    }
    
    
}
