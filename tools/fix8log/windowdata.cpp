#include "windowdata.h"
WindowData::WindowData():currentTab(0)
{

}
WindowData::WindowData(const WindowData &w)
{
    id       = w.id;
    geometry = w.geometry;
    state    = w.state;
    color    = w.color;
    currentTab = w.currentTab;
}

