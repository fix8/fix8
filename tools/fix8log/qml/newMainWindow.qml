import QtQuick 2.1
import QtQuick.Controls 1.2
import Qt.labs.folderlistmodel 2.1
import QtQuick.Dialogs 1.0



Rectangle {
    id:appWindow
    objectName: "appWindow"
    width:640
    height:480
    Component {
        id: filePage

        Rectangle {
            id:filePageRect
            objectName: "filePage"
            anchors.fill: parent
            Label {
                id:fileTitleID
                color:"black"
                font.bold: true;
                font.italic: true
                text:"Create New Window"
                anchors.top: parent.top
                anchors.topMargin: 13
                anchors.horizontalCenter: parent.horizontalCenter
                font.pointSize: 18
            }
            Rectangle {
                id:hrLineFile
                color:"black"
                height:2
                anchors.top: fileTitleID.bottom
                anchors.topMargin: 5
                anchors.right: parent.right
                anchors.left: parent.left
            }
            Rectangle {
                id: fileArea
                radius: 6
                border.color: "#444444"
                anchors.left: parent.left
                anchors.leftMargin: 7
                anchors.right: parent.right
                anchors.rightMargin: 7
                anchors.top:  hrLineFile.bottom
                anchors.topMargin: 27
                anchors.bottom: backButton.top
                anchors.bottomMargin: 13
                color:"white"

            }
            FileDialog {
                id: fileDialog
                title: "Please choose a file"
            }
            Button {
                id:backButton
                text: "Back"
                tooltip: "Next select log file to open."
                anchors.right: parent.right
                anchors.rightMargin: 15
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 13
                onClicked: {
                    //stackView.pop(filePage)
                    fileDialog.visible = true
                }
            }
        }
    }
    StackView {
        id: stackView
        anchors.fill: parent;
        initialItem: schemaPage
        delegate: StackViewDelegate {
            function transitionFinished(properties)
            {
                properties.exitItem.opacity = 1
            }
            pushTransition: StackViewTransition {
                PropertyAnimation {
                    target: enterItem
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 650
                }
                PropertyAnimation {
                    target: exitItem
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 600
                }
            }
        }
        Component {
            id: schemaPage
            Rectangle {
                id:schemaPageRect
                objectName: "schemaPage"
                property alias fontPointSize: instructions.font.pointSize
                anchors.fill: parent
                Label {
                    id:titleID
                    color:"black"
                    font.bold: true;
                    font.italic: true
                    text:"Create New Window"
                    anchors.top: parent.top
                    anchors.topMargin: 13
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 18
                }
                Rectangle {
                    id:hrLine
                    color:"black"
                    height:2
                    anchors.top: titleID.bottom
                    anchors.topMargin: 5
                    anchors.right: parent.right
                    anchors.left: parent.left
                }
                Rectangle {
                    id: schemaArea
                    radius: 6
                    border.color: "#444444"
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    anchors.top:  hrLine.bottom
                    anchors.topMargin: 27
                    anchors.bottom: nextButton.top
                    anchors.bottomMargin: 13
                    width:170
                    color:"white"
                    ListView {
                        id: schemaListView
                        anchors.fill:parent
                    }
                }
                TextArea {
                    id:instructions
                    anchors.right: parent.right
                    anchors.rightMargin: 21
                    anchors.top: hrLine.bottom
                    anchors.topMargin: 27
                    anchors.bottom: nextButton.top
                    anchors.bottomMargin: 13
                    anchors.left: schemaArea.right
                    anchors.leftMargin: 21
                    textFormat: TextEdit.RichText
                    text: "Select the FIX  schema from this list.  These schemas are found in the application folder in the \"fixschemas\" subdirectory, and also in the use's home directory in the \"logview/fixschemas\" subdirecty. <p> If you do not see your shema in this list plesae copy it to one the two subdirectoires mentioned."
                    font.bold: true
                    backgroundVisible:false
                    textColor:"black"
                    textMargin: 13
                    frameVisible: false
                }
                Button {
                    id:nextButton
                    text: "Next"
                    tooltip: "Next select log file to open."
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 13
                    onClicked: {
                        stackView.push(filePage)
                    }
                }
            }
        }
    }
}

