#include <proxyFilter.h>
#include <QDebug>
#include <QtScript>

int ProxyFilter::senderIDRole = Qt::UserRole+2;

ProxyFilter::ProxyFilter(QObject *parent): QSortFilterProxyModel(parent),logicFilter(0),filterFunction(0)
{
    rowAccepted =  false;

}
void ProxyFilter::setFilterFunction(const SearchFunction *ff)
{
    filterFunction = ff;
    qDebug() << "PROXY FILTER SET TO " << filterFunction->javascript;
}

void ProxyFilter::setAcceptedSendIDs(QStringList sendIDs)
{
    senderIDs = sendIDs;
}
bool ProxyFilter::filterAcceptsRow (int row, 
                                    const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    rowAccepted =  true;
    bool acceptSymbol    = true;
    bool bstatus;
    QStandardItemModel *sm =
            (QStandardItemModel *) sourceModel();
    QModelIndex mi = sm->index(row,0);
    QString str = mi.data(senderIDRole).toString();
    if (str.length() < 1)
        acceptSymbol = false;
    else if(senderIDs.contains(str))
        acceptSymbol = false;
    if (!acceptSymbol) {
       rowAccepted =  false;
       return false;
     }

    QModelIndex mi1;
     QVariant    var;
     QMap<QString, qint16>::iterator iter;
     QMap<QString, qint16> tempMap(columnMap);
     QString key;
     QList <QVariant> list;
     for (iter = tempMap.begin();
          iter != tempMap.end(); iter++) {
       qint16 column = iter.value();
       mi1 = sm->index(row,column);
       if (!mi1.isValid())  {
         qWarning() << "Invalid model index for logic filter " << __FILE__;
         return true;
       }
       var = mi1.data(Qt::UserRole+1);
       list.append(var);
     }
     bstatus = true;
     QListIterator <QVariant> iter2(list);
     //bstatus = processLogicFilter(list);
     return  bstatus;
}

bool ProxyFilter::isRowAccepted()
{
    qDebug() << "IS ROW ACCEPETED = " << rowAccepted << __FILE__ << __LINE__;
    return rowAccepted;
}
void ProxyFilter::setLogicFilter(LogicFilter *lf)
{
    logicFilter = lf;
}
bool ProxyFilter::processLogicFilter(QList <QVariant> &values) const
{
  QScriptValueList args;
  QScriptValueList args2;
  QString str;
  QString strValue;
  QMap<QString, qint16> tempMap(columnMap);

  QListIterator<QVariant> iter2(values);
  QVariant var;
  iter2.toFront();
  while(iter2.hasNext()) {
    var = iter2.next();
    switch (var.type()) {
    case QVariant::Int:
    case QVariant::UInt:
      args << var.toInt();
      break;
    case QVariant::Double:
      args << var.toDouble();
      break;
    case QVariant::String:
      args << var.toString();
      break;
    default:
      qWarning() << "Invalid varaint " << __FILE__ << __LINE__;
    }


  }
  QScriptValue qs;
  if (args.count() < 1) {
    qWarning() << "Invalid args " << __FILE__ << __LINE__;
    return true;
  }
  QScriptValue result =fun.call(fun,args);

  return result.toBool();
}
void ProxyFilter::setLogicColumnMap(QMap <QString, qint16> &cm)
{
  columnMap = cm;

}
