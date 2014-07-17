import QtQuick 2.0

Rectangle {
    id: fieldInfo
    objectName: "fieldInfoArea"
    color:"green"
    anchors.fill: parent
    Text {
        id:title
        text: "Fields"
        anchors.leftMargin: 5
        anchors.topMargin: 8
        font.bold: true
        font.pointSize: 12
        horizontalAlignment: Text.AlignLeft
        anchors.left:parent.left;
        anchors.top:parent.top
    }
    Rectangle {
        id: workArea
        color: fieldInfo.color
        anchors.top: title.bottom
        anchors.topMargin: 12
        anchors.bottom:parent.bottom
        anchors.bottomMargin: 12
        clip: true
        anchors.right: parent.right
        anchors.left: parent.left
        ListModel {
            id:letterModel
            ListElement {
                name : "A-B"
            }
            ListElement {
                name : "C-D"
            }
            ListElement {
                name : "E-F"
            }
            ListElement {
                name : "G-H"
            }
            ListElement {
                name : "I-J"
            }
            ListElement {
                name : "K-L"
            }
            ListElement {
                name : "L"
            }
            ListElement {
                name : "M-N"
            }
            ListElement {
                name : "O-P"
            }
            ListElement {
                name : "Q-R"
            }
            ListElement {
                name : "S-T"
            }
            ListElement {
                name : "U-V"
            }
            ListElement {
                name : "W-X"
            }
            ListElement {
                name : "Y-Z"
            }
        }
        Component {
            id: setupDelegate
            Item {
                id: wrapper
                width:80
                height:28
                MouseArea{
                    anchors.fill: parent
                    z: 1
                    hoverEnabled: true
                    onPressed:  {
                            letterArea.currentIndex = index
                            letterArea.forceActiveFocus()
                    }
                }
                Rectangle{
                    id:item
                    property color gradColor1: "#aaaaaa"
                    property color gradColor2: "black"
                    gradient: Gradient {
                               id:buttonGrad
                              GradientStop {  position: 0.0; color: item.gradColor1 }
                              GradientStop {  position: 1.0; color: item.gradColor2 }
                          }
                    border.width:2
                    border.color: "#666666"
                    radius: 5
                    anchors.fill: parent
                    anchors.margins: 2
                    Text {
                        anchors.centerIn: parent
                        color:"white"
                        font.bold:true
                        text: name
                    }
                }
                // indent the item if it is the current item
                states: State {
                    name: "Current"
                    when: wrapper.ListView.isCurrentItem
                    PropertyChanges { target: item; gradColor1:"lightblue"; gradColor2: "darkblue"; border.color:"darkblue"; border.width:3}
                }
                transitions: Transition {
                    NumberAnimation { properties: "y"; duration: 400 }
                }
            }
        }
        ListView {
            id:letterArea
            focus: true
            anchors.top:paarent.top
            anchors.topMargin: 13
            anchors.left:parent.left
            width:90
            height:400
            model: letterModel
            highlightFollowsCurrentItem: false
            boundsBehavior: Flickable.StopAtBounds
            keyNavigationWraps: true
            delegate: setupDelegate
            clip: true

        }
        Rectangle {
            id:money
            anchors.left : letterArea.right
            anchors.leftMargin: 24
            anchors.right: parent.right
            anchors.rightMargin:7
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            color:"#bbbbbb"
        }
    }
    function setBackground(bgColor)
    {
        color = bgColor;
    }
}
