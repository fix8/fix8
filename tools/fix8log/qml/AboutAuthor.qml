import QtQuick 2.0

Item {
    id: aboutMe
    anchors.fill: parent
    Rectangle {
        anchors.fill: parent
        Text {
            id:title
            text: "Author"
            color:"black"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

}
