import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
ColumnLayout  {
    id: columnLayout1
    width: 400
    height:400
    spacing: 0
    property alias label: tabLabel.text
    Image {
        id: tabImage
        source: "../images/32x32/tab.png"
        Label {
            id: tabLabel
            anchors.centerIn: parent
            text: "A-B"
            color: "white"
            horizontalAlignment: Text.AlignLeft
            font.bold: true
        }
    }
    Rectangle {
        id:line
        color:"darkblue"
        transformOrigin: Item.Center
        height:2
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
    }
    Rectangle {
        Layout.alignment: Qt.AlignRight

        Layout.fillHeight: true

        Layout.fillWidth: true
        color: "white"
        anchors.top: line.bottom
        anchors.topMargin: 5
        Layout.preferredHeight: 50
        Layout.minimumHeight: 30
        ListModel {
            id: fieldModel
            ListElement {
                name: "Jim Williams"
            }
            ListElement {
                name: "John Brown"
            }
            ListElement {
                name: "Bill Smyth"
            }
            ListElement {
                name: "Price"
            }
            ListElement {
                name: "Proceess"
            }
            ListElement {
                name: "P Function"
            }
            ListElement {
                name: "P Factor"
            }
            ListElement {
                name: "R Value"
            }
        }
        GridView {
            anchors.fill:parent

            model: fieldModel
            cellHeight:24
            delegate: Column {
                CheckBox { text: name}
            }
        }
    }
}
