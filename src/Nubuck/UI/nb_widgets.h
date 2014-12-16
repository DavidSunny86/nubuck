#pragma once

#include <QPushButton>

class NBW_Button : public QPushButton {
    Q_OBJECT
private:
    unsigned _id;
private slots:
    void OnClicked();        
public:
    NBW_Button(unsigned id, const char* name);
};