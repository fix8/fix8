/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

#include "database.h"
#include "fix8log.h"
#include "fixmimedata.h"
#include "globals.h"
#include "mainwindow.h"
#include "messagefield.h"
#include "schemaeditordialog.h"
#include "worksheetmodel.h"
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QtWidgets>
#include "worksheetmodel.h"
#include <fix8/f8includes.hpp>
#include "fix8/field.hpp"
#include "fix8/message.hpp"
#include <fix8/f8types.hpp>
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
#include <iostream>
#include <string.h>
using namespace GUI;
using namespace FIX8;
using namespace std;

void Fix8Log::generate_traits(const TraitHelper& tr,QMap <QString, QBaseEntry *> &baseMap,FieldUseList &ful,
                              MessageField *mf,QList <QBaseEntry *> *qbaseEntryList,int *level)
{
    int ii = 0;
    for (F8MetaCntx::const_iterator itr(F8MetaCntx::begin(tr)); itr != F8MetaCntx::end(tr); ++itr)
    {
        QBaseEntry *qbe;
        FieldUse *fieldUse = 0;
        QString name;
        const BaseEntry *be(TEX::ctx().find_be(itr->_fnum)); // lookup the field
        if(qbaseEntryList) {
            qbe = baseMap.value(be->_name);
            if (!qbe) {
                qbe  = new QBaseEntry(*be);
                qbe->ft = new FieldTrait(*itr);
                baseMap.insert(qbe->name,qbe);
            }
            name = qbe->name;
            qbaseEntryList->append(qbe);
            if (defaultHeaderStrs.contains(name)) {
                if (!defaultHeaderItems.findByName(name))
                    defaultHeaderItems.append(qbe);
            }
            fieldUse = ful.findByName(name);
            if (!fieldUse) {
                fieldUse = new FieldUse();
                fieldUse->name = name;
                fieldUse->field = qbe->ft;
                ful.append(fieldUse);
            }
            fieldUse->messageFieldList.append(mf);
        }
        else
            qWarning() << "\t\tERROR QBASELIST = 0" ;
        //MessageBase *header =  new Message::Header();
        //cout << "Field Type: " << ft._ftype << endl;
        //cout << spacer << "\t" << *itr << endl; // use FieldTrait insert operator. g out traits.
        if (itr->_field_traits.has(FieldTrait::group)) {// any nested repeating groups?
            qbe->baseEntryList = new QList<QBaseEntry *>();
            generate_traits(itr->_group,baseMap,ful,mf,qbe->baseEntryList,level); // descend into repeating groups
        }
        ii++;
    }
}
void Fix8Log::generate_traits(const TraitHelper& tr,QMap <QString, QBaseEntry *> &baseMap,FieldUseList &ful,
                              MessageField *mf,QBaseEntryList *qbaseEntryList,int *level)
{
    int ii = 0;
    QString name;
    for (F8MetaCntx::const_iterator itr(F8MetaCntx::begin(tr)); itr != F8MetaCntx::end(tr); ++itr)
    {
        QBaseEntry *qbe;
        FieldUse *fieldUse = 0;
        const BaseEntry *be(TEX::ctx().find_be(itr->_fnum)); // lookup the field
        if(qbaseEntryList) {
            qbe = baseMap.value(be->_name);
            if (!qbe) {
                qbe  = new QBaseEntry(*be);
                qbe->ft = new FieldTrait(*itr);
                baseMap.insert(qbe->name,qbe);
            }
            name = qbe->name;
            qbaseEntryList->append(qbe);
            fieldUse = ful.findByName(name);
            if (!fieldUse) {
                fieldUse = new FieldUse();
                fieldUse->name = name;
                fieldUse->field = qbe->ft;
                ful.append(fieldUse);
            }
            fieldUse->messageFieldList.append(mf);
            if (defaultHeaderStrs.contains(name)) {
                if (!defaultHeaderItems.findByName(name))
                    defaultHeaderItems.append(qbe);
            }
        }
        if (itr->_field_traits.has(FieldTrait::group)) {// any nested repeating groups?
            qbe->baseEntryList = new QList<QBaseEntry *>();
            (*level)++;
            generate_traits(itr->_group,baseMap,ful,mf,qbe->baseEntryList,level); // descend into repeating groups
        }
        ii++;
    }
}
