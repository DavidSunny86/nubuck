#pragma once

#include <QPushButton>
#include <QCheckBox>

class NBW_Button : public QPushButton {
    Q_OBJECT
private:
    unsigned _id;
private slots:
    void OnClicked();        
public:
    NBW_Button(unsigned id, const char* name);
};

class NBW_CheckBox : public QCheckBox {
    Q_OBJECT
private:
    unsigned _id;
private slots:
    void OnToggled(bool isChecked);
public:
    NBW_CheckBox(unsigned id, const char* name);
};