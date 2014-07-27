#include "searchfunction.h"
#include <QDebug>

SearchFunction::SearchFunction():id(-9999)
{
}
SearchFunction::SearchFunction(const SearchFunction &sf)
{
    id = sf.id;
    alias = sf.alias;
    function = sf.function;
    javascript = sf.javascript;
}
SearchFunction & SearchFunction::operator=( const SearchFunction &rhs)
{
    if (this == &rhs)
        return *this;
    id = rhs.id;
    alias = rhs.alias;
    function = rhs.function;
    javascript = rhs.javascript;
    return *this;
}
bool SearchFunction::operator==(const SearchFunction &rhs)
{
    if (this == &rhs)
        return true;
    if ((id == rhs.id) &&(alias == rhs.alias) && (function== rhs.function))
        return true;
    else
        return false;
}
SearchFunctionList::SearchFunctionList(): QList<SearchFunction *>()
{

}
SearchFunction *SearchFunctionList::findByID(qint32 id)
{
    SearchFunction *sf = 0;
    if (count() < 1)
        return sf;
    QListIterator <SearchFunction *> iter(*this);
    while(iter.hasNext()) {
        sf = iter.next();
        if (sf->id == id)
            return sf;
    }
    return 0;
}
SearchFunctionList & SearchFunctionList::operator=( const SearchFunctionList &rhs)
{
    SearchFunction *sf;
    SearchFunction *oldSF;
    if (this == &rhs) {
        return *this;
    }
  this->clear();
  QListIterator <SearchFunction *> iter(rhs);
  while(iter.hasNext()) {
      oldSF = iter.next();
      sf = new SearchFunction(*oldSF);
      append(sf);
  }
  return *this;
}

