#include "fix8sharedlib.h"

Fix8SharedLib::Fix8SharedLib():count(0)
{
}

 Fix8SharedLibList::Fix8SharedLibList(): QList<Fix8SharedLib *>()
 {

 }
 Fix8SharedLib * Fix8SharedLibList::findByName(QString  &name)
 {
  Fix8SharedLib  *fsl = 0;
  QListIterator <Fix8SharedLib *> iter(*this);
  while(iter.hasNext()) {
    fsl = iter.next();
    if (fsl->name == name)
        return(fsl);
  }
  return(0);
 }
bool Fix8SharedLibList::removeByName(QString &name)
{
    bool bstatus = false;
    Fix8SharedLib  *fsl = 0;
    QList<Fix8SharedLib *>::iterator iter;
    for(iter = this->begin();iter != this->end();++iter) {
       fsl = *iter;
       if (fsl->name == name) {
           erase(iter);
           bstatus = true;
           break;
       }
    }
    return bstatus;
}
