#pragma once

#include <QDockWidget>
#include "ui_operatorpanelheader.h"

namespace UI {

class OperatorPanel : public QDockWidget {
private:
    Ui::OperatorPanelHeader _headerUi;
    QWidget*                _header;
public:
    OperatorPanel(QWidget* parent = NULL);

    void SetOperatorName(const QString& name);

    void setWidget(QWidget* widget);

    void Clear() {
        setWidget(NULL);
        setWindowTitle("Operator (None)");
    }
};

} // namespace UI