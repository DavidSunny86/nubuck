#pragma once

#include <QDialog>

#include "ui_faceconfig.h"

namespace UI {

    class FaceConfig : public QDialog {
        Q_OBJECT
    private:
        Ui::FaceConfig _ui;
    private slots:
        void OnSpeedChanged(double val);
        void OnDecalSizeChanged(double val);
        void OnSpacingChanged(double val);
        void OnCurvatureChanged(double val);
    public:
        FaceConfig(QWidget* parent = NULL); // Qt dictates public ctor

        static void Show(void) {
            static FaceConfig* instance = NULL;
            if(!instance) instance = new FaceConfig();
            instance->show();
        }
    };

} // namespace UI