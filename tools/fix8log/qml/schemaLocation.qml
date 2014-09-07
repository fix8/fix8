import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
Rectangle {
    id: aboutSchemas
    objectName:"aboutSchemas"
    color: backgroundcolor
    width: 600
    height: 550

    Rectangle {
        id: rectangle1
        anchors.fill: parent
        color: "#ffffff"
        radius: 26
        border.color: "orange"
        border.width: 3
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 36
        anchors.top: parent.top
        anchors.topMargin: 35
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.rightMargin: 20
        anchors.leftMargin: 20
        Rectangle {
            id: page1
            anchors.fill: parent;
            anchors.margins: 9
            Text {
                id: title
                anchors.top:parent.top
                anchors.left: parent.left
                anchors.topMargin: 24
                anchors.leftMargin: 15
                width: 85
                height: 36
                text: qsTr("Schema Locations")
                font.pixelSize: 24
                font.italic: true
                font.bold: true
            }
            Text {
                id: intro
                anchors.top:title.bottom
                anchors.left: parent.left
                anchors.topMargin: 15
                anchors.leftMargin: 15
                anchors.right: parent.right
                anchors.rightMargin: 15
                height: 50
                text: qsTr("Each LogViewer window is assoicated with one <i>FIX Schema</i>. Available schemas are shown on the right. This list schemas is generated from the following two sources:")
                font.pixelSize: 16
                font.bold: true
                wrapMode: Text.Wrap
            }
            GridLayout {
                id:grid
                anchors.top:intro.bottom
                anchors.topMargin: 15
                anchors.left: parent.left
                anchors.leftMargin: 15
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 22
                columns: 2
                rows:2
                rowSpacing:21
                Label {
                    id:systemLabel
                    text:"System"
                    font.bold: true
                    font.pixelSize: 18
                }

                Label {
                    id: system
                    Layout.preferredWidth: 355
                    Layout.minimumHeight: 60
                    text: qsTr("<i>Fix8logviewer</i> ships with some of the common FIX Schemas. These schemas are located in.")
                    clip: true

                    font.pixelSize: 16
                    font.bold: true
                    wrapMode: Text.WordWrap
                }
                Label {
                    id:userLabel
                    text:"User"
                    font.bold: true
                    font.pixelSize: 18
                }
                Label {
                    id: user
                    Layout.minimumHeight: 60
                    Layout.preferredWidth: 355

                    text: qsTr("<i>Fix8logviewer</i> allows users to install their own FIX schema libs. These schemas may be added to the <i>System</i> schema directory or in the the directory \"<i>f8logviewer/schemalibs</i>\" located in the users's HOME directory.")
                    font.pixelSize: 16
                    font.bold: true
                    wrapMode: Text.Wrap
                }
            }
        }
        Rectangle {
            id: page2
            opacity:0
            anchors.fill: parent;
            anchors.margins: 9
            Text {
                id: title2
                anchors.top:parent.top
                anchors.left: parent.left
                anchors.topMargin: 24
                anchors.leftMargin: 15
                width: 85
                height: 36
                text: qsTr("Adding Schemas")
                font.pixelSize: 24
                font.italic: true
                font.bold: true
            }
            Label {
                id: advances
                textFormat:Text.RichText
                text: '<html> <body> Creating your own schemas is possible with the <b>FIX8 SDK </b>.  Please refer to the <a href="http://www.fix8.org"> www.fix8.org </a> website for requirements on how to do this.  Once a new Schema library is created you can put it in the <i>System\'s</i> directory to make it available to all users, or place in a <i>User\'s</i> directory for local access only. </body></html>'
                onLinkActivated:  Qt.openUrlExternally("http://www.fix8.org")
                anchors.top: title2.bottom
                anchors.topMargin: 32
                anchors.left: parent.left
                anchors.leftMargin:8
                anchors.right: parent.right
                anchors.rightMargin:8
                anchors.bottom: parent.bottom
                font.pixelSize: 16
                wrapMode: Text.Wrap

                MouseArea {
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                    anchors.fill: parent
                    onEntered: {
                        cursorShape:Qt.WaitCursor
                    }

                }
            }


        }

        states: [
            State {
                name: "AddingSchemas"
                PropertyChanges { target: page1; opacity: 0}
                PropertyChanges { target: page2; opacity: 100}
            }
        ]
        transitions: [
                 Transition {
                     from: ""
                     to: "AddingSchemas"
                     SequentialAnimation {
                     PropertyAnimation { target: page1; property: "opacity";}
                     PropertyAnimation { target: page2; property: "opacity"; easing.type: Easing.InOutQuad; duration:1400}
                     }
                 },
                 Transition {
                     from: "AddingSchemas"
                     to: ""
                      SequentialAnimation {
                     PropertyAnimation { target: page2; property: "opacity"; }
                     PropertyAnimation { target: page1; property: "opacity"; easing.type: Easing.InOutQuad; duration:1400}
                    }
                }
             ]
    }
    Button {
        id :button

        text: rectangle1.state == "" ? "Adding Schemas..." : "Go Back"
        onClicked: {
            if (rectangle1.state == "")
                rectangle1.state = "AddingSchemas"
            else
                rectangle1.state = "";
        }
        style: ButtonStyle {
            background: Rectangle {
                id: btnBackground
                implicitWidth: 124
                border.width: control.activeFocus ? 2 : 1
                border.color: "grey"
                color: "orange"
                radius: 6
            }
            label: Text {
                id: btnText
                text: button.text
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                font.bold: true

            }
        }
        anchors.top: rectangle1.bottom
        anchors.topMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 20
    }

}

