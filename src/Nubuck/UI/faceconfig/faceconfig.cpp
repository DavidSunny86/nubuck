#include <world\entities\ent_face\ent_face.h>
#include "faceconfig.h"

namespace UI {

    void FaceConfig::OnDecalSizeChanged(double val) {
        W::cvar_faceDecalSize = (float)val;
    }

    FaceConfig::FaceConfig(QWidget* parent) : QDialog(parent) {
        _ui.setupUi(this);

        _ui.sbDecalSize->setValue(W::cvar_faceDecalSize);
    }

} // namespace UI