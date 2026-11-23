import QtQuick
import QtLocation
import QtPositioning
import CommutePlanner

Item {
    id: root

    required property CityViewModel viewModel

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
        map.center: QtPositioning.coordinate(64.0, 26.0)
        map.zoomLevel: 5
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
    }

    MapItemView {
        parent: view.map
        model: root.viewModel.cities
        delegate: MapQuickItem {
            id: marker
            required property int index
            required property real latitude
            required property real longitude
            required property int value
            required property color markerColor

            coordinate: QtPositioning.coordinate(latitude, longitude)
            anchorPoint.x: bubble.width / 2
            anchorPoint.y: bubble.height / 2

            sourceItem: Rectangle {
                id: bubble
                width: 36
                height: 36
                radius: width / 2
                color: marker.markerColor
                border.color: "white"
                border.width: 2

                Text {
                    anchors.centerIn: parent
                    text: marker.value
                    color: "white"
                    font.bold: true
                    font.pixelSize: 14
                    style: Text.Outline
                    styleColor: Qt.rgba(0, 0, 0, 0.35)
                }

                TapHandler {
                    onTapped: root.viewModel.selectCity(marker.index)
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
