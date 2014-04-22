#pragma once

#include <QTextStream>
#include <Nubuck\generic\pointer.h>
#include "ui_console.h"

namespace UI {

class PlainTextDevice;

class Console : public QWidget {
    Q_OBJECT
private:
    Ui::Console                     _ui;
    GEN::Pointer<PlainTextDevice>   _textDevice;
    QTextStream                     _textStream;
public slots:
    void OnExec();
public:
    Console(QWidget* parent = NULL);
};

} // namespace UI