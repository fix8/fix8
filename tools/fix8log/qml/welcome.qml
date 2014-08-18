import QtQuick 2.0

Rectangle {
    id:root
    width:750
    height:550

    Rectangle {
        id:leftSide
        anchors.left:parent.left
        anchors.bottom: parent.bottom;
        anchors.top: parent.top
        width:200
        color: "#bf5b18"
        Rectangle {
            id:fix8info
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 7
            anchors.right: parent.right
            anchors.rightMargin: 7
            height:100
            color: "#bf5b18"
            Text {
                id:leftTitle
                color:"white"
                font.bold: true
                font.pointSize: 22
                text: "Fix8"
                horizontalAlignment: Text.AlignHCenter
                anchors.top: parent.top
                anchors.topMargin: 7
                anchors.left: parent.left
                anchors.right: parent.right
            }
            Rectangle {
                id:leftHorLine
                color:"white"
                anchors.left: parent.left
                anchors.right: parent.right
                height:2
                anchors.top: leftTitle.bottom
                anchors.topMargin: 7
            }
            Text {
                id:url
                color:"white"
                font.bold: true
                font.italic: true
                font.pointSize: 14
                text: "www.fix8.org"
                horizontalAlignment: Text.AlignHCenter
                anchors.top: leftHorLine.bottom
                anchors.topMargin: 7
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }

    }

    Rectangle {
        id: rightSide
        anchors.left: leftSide.right
        anchors.bottom: parent.bottom
        anchors.top:parent.top
        anchors.right:parent.right
        color: "white"
        Rectangle {
            id:textRect
           anchors.top:parent.top
           anchors.topMargin:64
           anchors.left:parent.left
           anchors.leftMargin:32
           anchors.right:parent.right
           anchors.rightMargin:32
           anchors.bottom:parent.bottom

            Text {
                id:infoMessage
                anchors.left:parent.left
                anchors.right:parent.right
                anchors.top:parent.top
                anchors.bottom:parent.bottom
                wrapMode:Text.Wrap
                text: "The Fix8 LogViewer enables you to have multiple windows.  Each window needs to be assocated with a FIX schema. There are several schemas built in but you may also add your own schema libraries<p>To create your first window, and any new window afterwards you must first select your <i>schema</i>.  Hit the <b>Next</b> to select your <i>schema</i>."
                font.pointSize: 18
                textFormat: Text.RichText
            }
        }

    }
}
