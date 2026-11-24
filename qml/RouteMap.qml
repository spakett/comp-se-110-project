import QtQuick
import QtLocation
import QtPositioning
import CommutePlanner

Item {
    id: root

    required property RouteViewModel viewModel

    Plugin {
        id: tilePlugin
        name: "osm"
        PluginParameter { name: "osm.useragent"; value: "CommutePlanner/0.1 (TUNI COMP.SE.110 course project)" }
        PluginParameter { name: "osm.mapping.providersrepository.disabled"; value: true }
        PluginParameter { name: "osm.mapping.custom.host"; value: "https://tile.openstreetmap.org/" }
    }

    MapView {
        id: view
        anchors.fill: parent
        map.plugin: tilePlugin
        map.center: QtPositioning.coordinate(60.19, 24.94)
        map.zoomLevel: 11
        map.copyrightsVisible: false
        minimumZoomLevel: 4
        maximumZoomLevel: 17

        Component.onCompleted: {
            for (const type of map.supportedMapTypes) {
                if (type.style === MapType.CustomMap) {
                    map.activeMapType = type
                    break
                }
            }
        }

        Timer {
            id: viewportTimer
            interval: 250
            onTriggered: {
                const box = view.map.visibleRegion.boundingGeoRectangle()
                root.viewModel.reportViewport(box.topLeft.latitude, box.bottomRight.latitude,
                                              box.topLeft.longitude, box.bottomRight.longitude)
            }
        }
        Connections {
            target: view.map
            function onCenterChanged() { viewportTimer.restart() }
            function onZoomLevelChanged() { viewportTimer.restart() }
            function onWidthChanged() { viewportTimer.restart() }
            function onHeightChanged() { viewportTimer.restart() }
        }

        TapHandler {
            onTapped: (eventPoint) => {
                const coord = view.map.toCoordinate(eventPoint.position)
                root.viewModel.pickPoint(coord.latitude, coord.longitude)
            }
        }
    }

    MapItemView {
        parent: view.map
        model: root.viewModel.heatmap
        delegate: MapRectangle {
            required property real north
            required property real south
            required property real west
            required property real east
            required property color cellColor

            topLeft: QtPositioning.coordinate(north, west)
            bottomRight: QtPositioning.coordinate(south, east)
            color: cellColor
            border.width: 0
        }
    }

    MapPolyline {
        parent: view.map
        line.width: 5
        line.color: "#2d6cdf"
        path: root.viewModel.routePath
    }

    MapQuickItem {
        parent: view.map
        visible: root.viewModel.startPoint.isValid
        coordinate: root.viewModel.startPoint
        anchorPoint.x: startPin.width / 2
        anchorPoint.y: startPin.height / 2
        sourceItem: Rectangle {
            id: startPin
            width: 28
            height: 28
            radius: width / 2
            color: "#2e8b57"
            border.color: "white"
            border.width: 2
            Text { anchors.centerIn: parent; text: "A"; color: "white"; font.bold: true; font.pixelSize: 14 }
        }
    }

    MapQuickItem {
        parent: view.map
        visible: root.viewModel.destinationPoint.isValid
        coordinate: root.viewModel.destinationPoint
        anchorPoint.x: destPin.width / 2
        anchorPoint.y: destPin.height / 2
        sourceItem: Rectangle {
            id: destPin
            width: 28
            height: 28
            radius: width / 2
            color: "#d64545"
            border.color: "white"
            border.width: 2
            Text { anchors.centerIn: parent; text: "B"; color: "white"; font.bold: true; font.pixelSize: 14 }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 8
        width: legend.implicitWidth + 12
        height: legend.implicitHeight + 8
        color: "white"

        Row {
            id: legend
            anchors.centerIn: parent
            spacing: 8
            Repeater {
                model: ["Good", "Fair", "Moderate", "Poor", "Very poor", "Extremely poor"]
                delegate: Row {
                    required property string modelData
                    required property int index
                    spacing: 3
                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 10
                        height: 10
                        color: root.viewModel.colorForLevel(index)
                    }
                    Text { text: modelData; font.pixelSize: 11 }
                }
            }
        }
    }

    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        text: "© <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors"
        textFormat: Text.RichText
        font.pixelSize: 10
        onLinkActivated: (link) => Qt.openUrlExternally(link)
    }
}
