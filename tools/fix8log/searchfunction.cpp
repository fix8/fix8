#include "searchfunction.h"

SearchFunction::SearchFunction():id(-9999)
{
}
SearchFunction::SearchFunction(const SearchFunction &sf)
{
    id = sf.id;
    alias = sf.alias;
    function = sf.function;
}
SearchFunction & SearchFunction::operator=( const SearchFunction &rhs)
{
    if (this == &rhs)
        return *this;
    id       = rhs.id;
    alias    = rhs.alias;
    function = rhs.function;
    return *this;
}

SearchFunctionList::SearchFunctionList(): QList<SearchFunction>()
{

}
