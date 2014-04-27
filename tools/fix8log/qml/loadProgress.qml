import QtQuick 2.0
import QtQuick.Controls 1.1
Rectangle {
    id: progressID
    width: 600
    height:400
    color:"black"
    state: "loading"
    signal cancel()
    function doCancel() {
        cancel()
    }
    function  setLoadFile(str1,str2)
    {
        message.text = str1;
        message2.text = str2;
        state = "loadingfile"
    }

    function setMessage(str) {
        message.text = str;
    }
    function setFinished() {
        state = "finished"
    }
    function started() {
        state = "loading"
    }
    Text {
        id:title
        font.bold: true;
        font.italic: true
        color:"white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 12
        text:"Restoring Session"
    }
    Rectangle {
        id: horLine
        anchors.top:title.bottom
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 6
        height:2
        color:"white"
    }

    BusyIndicator {
        id:busyID
        width: 75
        height:75
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top:horLine.bottom
        anchors.topMargin: 75
        opacity: 1.0
    }
    Text {
        id:message
        font.bold: true;
        font.italic: true
        color:"white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: busyID.bottom
        anchors.topMargin: 75
    }
    Text {
        id:message2
        font.bold: true;
        font.italic: true
        color:"white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: message.bottom
        anchors.topMargin: 22
    }
    Button {
        id:cancelButton
        anchors.right:parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.rightMargin: 12
        text: "Cancel"
        tooltip: "Cancel Session Restore"

      onClicked: {
          console.log("Cancel Clicked")
          progressID.doCancel()
      }
    }

    states: [
        State {
            name: "loading"
            PropertyChanges { target: busyID; running:true}
            PropertyChanges { target:message2; opacity: 0.0}
            },
        State {
            name: "loadingfile"
            PropertyChanges { target: busyID; running:true}
            PropertyChanges { target:title; opacity: 0.0}
             PropertyChanges { target:horLine; opacity: 0.0}
             PropertyChanges { target:message2; opacity: 1.0}
            },
        State {
            name:"finished"
            PropertyChanges { target:busyID; running:false; opacity: 0.0}
            PropertyChanges {  target: message; opacity:0.0 }
            PropertyChanges {  target: progressID; opacity:0.0 }
            }
    ]
}
