#pragma once

#include <QDialog>

#include "ui_nodeconfig.h"

namespace UI {

    class NodeConfig : public QDialog {
        Q_OBJECT
    private:
        Ui::NodeConfig _ui;
    private slots:
        void OnSizeChanged(double val);
        void OnSubdivisionsChanged(int val);
    public:
        NodeConfig(QWidget* parent = NULL);
    };

} // namespace UI