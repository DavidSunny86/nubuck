#pragma once

#include <QDockWidget>

namespace UI {

class OperatorPanel : public QDockWidget {
public:
    OperatorPanel(QWidget* parent = NULL);

    static OperatorPanel* Instance() {
        static OperatorPanel* instance = NULL;
        if(!instance) instance = new OperatorPanel();
        return instance;
    };

    void Clear() {
        if(widget()) delete widget();
        setWidget(NULL);
        setWindowTitle("Operator (None)");
    }
};

} // namespace UI