#pragma once

#include <QWidget>
#include <QScrollArea>
#include "ui_operatorpanelheader.h"

namespace UI {

class OperatorPanel : public QWidget {
private:
    Ui::OperatorPanelHeader _headerUi;
    QWidget*                _header;
    QWidget*                _opWidget;
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