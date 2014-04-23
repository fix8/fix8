#include "windowdata.h"
WindowData::WindowData()
{

}
WindowData::WindowData(const WindowData &w)
{
    id       = w.id;
    geometry = w.geometry;
    state    = w.state;
    color    = w.color;
}

