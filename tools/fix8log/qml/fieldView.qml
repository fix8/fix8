import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
Rectangle {
    id: fieldInfo
    objectName: "fieldInfoArea"

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
            id:letterAreaView
            focus: true
            anchors.top:parent.top
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
            anchors.left : letterAreaView.right
            anchors.leftMargin: 24
            anchors.right: parent.right
            anchors.rightMargin:7
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            color:"lightgreen"

            ColumnLayout {
                id: fieldsArea
                anchors.fill: parent
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ListModel {
                        id:fieldsModel
                        ListElement {
                            name : "A-B"
                        }
                        ListElement {
                            name : "C-D"
                        }
                        ListElement {
                            name : "E-F"
                        }
                    }
                    Component {
                        id: fieldsAreaDelegate
                        ColumnLayout  {
                            id: columnLayout1
                            anchors.right: parent.right
                            anchors.left: parent.left

                            spacing:0
                            Image {
                                id: tabImage
                                source: "../images/32x32/tab.png"
                                Label {
                                    id: tabLabel
                                    anchors.centerIn: parent
                                    text: name
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
                    }

                    ListView {
                        anchors.fill: parent
                        id:fieldsByLetter
                        model:fieldsModel
                        delegate: fieldsAreaDelegate

                    }
                }
            }
        }
    }
    states: [
        State {
            name: "State1"
        }
    ]

    function setBackground(bgColor)
    {
        ; //color = bgColor;
    }
}
