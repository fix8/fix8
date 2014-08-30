#include <proxyFilter.h>
#include <QDebug>
#include <QtScript>

int ProxyFilter::senderIDRole = Qt::UserRole+2;

ProxyFilter::ProxyFilter(QObject *parent): QSortFilterProxyModel(parent),logicFilter(0)
{
  rowAccepted =  false;

}
void ProxyFilter::setAcceptedSendIDs(QStringList sendIDs)
{
  senderIDs = sendIDs;
}
bool ProxyFilter::filterAcceptsRow (int row, 
				    const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  bool found   = true;
  QStandardItemModel *sm = 
    (QStandardItemModel *) sourceModel();
    QModelIndex mi = sm->index(row,0);
    QString str = mi.data(senderIDRole).toString();
    if (str.length() < 1)
        found = false;
    else if(senderIDs.contains(str))
         found = false;
  return found;
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
