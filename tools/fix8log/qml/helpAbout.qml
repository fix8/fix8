import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.0

TabView {
    id:tv
    function serVersion(version)
    {
        versionText.text = "Version: " + version;
    }

    anchors.fill: parent
    Component.onCompleted: {
        addTab("Version",versionTab)
        addTab("Description", descriptionTab)
        addTab("Contact", contactTab)
        addTab("License", licenseTab)
    }
    Component {
        id: versionTab
        Rectangle {
            id:versionRec
            radius: 12
            anchors.fill: parent
            Text {
                id:versionText
                anchors.centerIn: parent
                font.pointSize: 16

            }
        }
    }
    Component {
        id: descriptionTab
        Rectangle {
            id:descriptionRec
            radius: 14
            anchors.fill: parent
            anchors.margins: 22
            TextArea {
                frameVisible: false
                id:textArea
                anchors.fill: parent
                anchors.margins: 6
                text: 'Fix8LogView is an opensource Fix log file viewer.  '
            }
        }
    }
    Component {
        id: contactTab
        Rectangle  {
            anchors.fill: parent
            anchors.margins: 22
            radius:12
            Text {
                id: name
                text: qsTr("Developer: David Boosalis")
                anchors.top: parent.top
                anchors.topMargin: 32
                anchors.left: parent.left
                anchors.leftMargin: 32
            }
            Text {
                id: email
                text: qsTr("Email:")
                anchors.top: name.bottom
                anchors.topMargin: 12
                anchors.left: parent.left
                anchors.leftMargin: 32

            }
            Text {
                id: emailAddress
                textFormat:Text.RichText
                text: '<html> <body> <a href="mailto://david.boosalis@gmail.com">david.boosalis@fix8.org</a></body></html>'
                onLinkActivated:  Qt.openUrlExternally("mailTo:david.booslis@gmail.com")
                font.italic: true
                font.bold: true
                color:"purple"
                anchors.top: name.bottom
                anchors.topMargin: 12
                anchors.left: email.right
                anchors.leftMargin:8
                MouseArea {
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                    anchors.fill: parent
                    onEntered: {
                        cursorShape:Qt.WaitCursor
                    }

                }
            }
            /**************/
            Text {
                id: fix8Community
                text: qsTr("Fix 8 Community:")
                anchors.top: emailAddress.bottom
                anchors.topMargin: 32
                anchors.left: parent.left
                anchors.leftMargin: 32
            }

            Text {
                id: fix8CommunityWeb
                textFormat:Text.RichText
                text: '<html> <body> <a href="http://www.fix8.org"> www.fix8.org</a> </body></html>'
                onLinkActivated:  Qt.openUrlExternally("http://www.fix8.org")
                font.italic: true
                font.bold: true
                color:"purple"
                anchors.top: emailAddress.bottom
                anchors.topMargin: 32
                anchors.left: fix8Community.right
                anchors.leftMargin:8
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
    }
    Component {
        id: licenseTab
        Rectangle {
            id:licRect
            radius: 12
            anchors.top: parent.top
            anchors.topMargin: 12
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
            TextArea {
                id:licText
                frameVisible: false
                anchors.fill: parent
                anchors.margins: 12
                textFormat: TextEdit.RichText
                text:  '<html> <body> This product is released under the <a href="https://www.gnu.org/licenses/quick-guide-gplv3.html">GNU LESSER GENERAL PUBLIC LICENSE Version 3 </a></body></html>'
                onLinkActivated:  Qt.openUrlExternally("https://www.gnu.org/licenses/quick-guide-gplv3.html")
            }
        }
    }


}

