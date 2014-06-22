#ifndef PROXY_FILTER_H
#define PROXY_FILTER_H

#include <QSortFilterProxyModel>
#include <QStringList>
#include <QtWidgets>
//#include <QtScript>

class ProxyFilter : public   QSortFilterProxyModel
{
  Q_OBJECT
 public:
  ProxyFilter(QObject *parent);
  bool isRowAccepted();
  void setAcceptedSendIDs(QStringList sendIDs);
  bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const ;
  static int senderIDRole;
private:
   QStringList senderIDs;
  mutable bool    rowAccepted;
};
#endif
