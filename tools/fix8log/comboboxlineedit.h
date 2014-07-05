#ifndef COMBOBOXLINEEDIT_H
#define COMBOBOXLINEEDIT_H

#include <QLineEdit>

class ComboBoxLineEdit : public QLineEdit
{
public:
    explicit ComboBoxLineEdit(QWidget *parent = 0);
protected:
    void focusOutEvent(QFocusEvent *);
};

#endif // COMBOBOXLINEEDIT_H
