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
    isVisible = w.isVisible;
    currentTab = w.currentTab;
    name       = w.name;
}

