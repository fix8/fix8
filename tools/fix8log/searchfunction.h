#ifndef SEARCHFUNCTION_H
#define SEARCHFUNCTION_H
#include <QString>
#include <QList>

class SearchFunction
{
public:
    SearchFunction();
    SearchFunction(const SearchFunction &);
    SearchFunction & operator=( const SearchFunction &rhs);
    qint32 id;
    QString alias;
    QString function;
};

class SearchFunctionList : public QList<SearchFunction>
{
public:
    SearchFunctionList();
};

#endif // SEARCHFUNCTION_H
