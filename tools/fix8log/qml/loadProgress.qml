//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

import QtQuick 2.0
import QtQuick.Controls 1.1
Rectangle {
    id: progressID
    width: 600
    height:400
    color:"black"
    state: "loading"
    signal cancel();
    function doCancel() {
        console.log("YES A CANCEL")
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
