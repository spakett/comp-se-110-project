import QtQuick
import QtLocation
import QtPositioning
import CommutePlanner

Item {
    id: root

    required property MapViewModel viewModel

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
        map.center: QtPositioning.coordinate(60.22, 24.90)
        map.zoomLevel: 10
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

    Connections {
        target: root.viewModel
        function onZoomRequested(steps) {
            view.map.zoomLevel = Math.max(view.minimumZoomLevel,
                                          Math.min(view.maximumZoomLevel, Math.round(view.map.zoomLevel) + steps))
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
