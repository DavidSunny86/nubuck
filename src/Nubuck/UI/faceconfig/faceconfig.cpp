#include <world\entities\ent_face\ent_face.h>
#include "faceconfig.h"

namespace UI {

    void FaceConfig::OnSpeedChanged(double val) { cvar_faceSpeed = (float)val; }
    void FaceConfig::OnDecalSizeChanged(double val) { cvar_faceDecalSize = (float)val; }
    void FaceConfig::OnSpacingChanged(double val) { cvar_faceSpacing = (float)val; }
    void FaceConfig::OnCurvatureChanged(double val) { cvar_faceCurvature = (float)val; }

    FaceConfig::FaceConfig(QWidget* parent) : QDialog(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint);

        _ui.setupUi(this);

        _ui.sbSpeed->setValue(cvar_faceSpeed);
        _ui.sbDecalSize->setValue(cvar_faceDecalSize);
        _ui.sbSpacing->setValue(cvar_faceSpacing);
        _ui.sbCurvature->setValue(cvar_faceCurvature);
    }

} // namespace UI