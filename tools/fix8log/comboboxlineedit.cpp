#include "comboboxlineedit.h"
#include <QDebug>
ComboBoxLineEdit::ComboBoxLineEdit(QWidget *parent) :
    QLineEdit(parent)
{

}
void ComboBoxLineEdit::focusOutEvent(QFocusEvent *)
{
    qDebug() << "Foucus out event";
}
