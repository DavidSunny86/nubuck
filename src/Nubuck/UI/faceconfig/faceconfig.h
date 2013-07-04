#pragma once

#include <QDialog>

#include "ui_faceconfig.h"

namespace UI {

    class FaceConfig : public QDialog {
        Q_OBJECT
    private:
        Ui::FaceConfig _ui;
    private slots:
        void OnDecalSizeChanged(double val);
    public:
        FaceConfig(QWidget* parent = NULL);
    };

} // namespace UI