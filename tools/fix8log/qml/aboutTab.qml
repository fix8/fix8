import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Rectangle {
    id:aboutArea
    width: 360
    height: 360

    TabView {
        id:tabArea
        anchors.fill: parent
        tabPosition:  Qt.TopEdge
        Tab {
            title:"Author"
            AboutAuthor { }
        }
        Tab {
            title:"License"
            AboutAuthor {

            }
        }
    }
}

