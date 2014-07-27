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
    bool operator==(const SearchFunction &rhs);
    qint32 id;
    QString alias;
    QString function;
    QString javascript;
};

class SearchFunctionList : public QList<SearchFunction *>
{
public:
    SearchFunctionList();
     SearchFunctionList & operator=( const SearchFunctionList &rhs);
    SearchFunction *findByID(qint32 id);
};

#endif // SEARCHFUNCTION_H
