#pragma once

#include <vector>
#include <LEDA\graph\node_map.h>
#include <Nubuck\editors\editor.h>

namespace NB {

class EntityEditor : public Editor {
private:
    struct ModeImpl {
        virtual ~ModeImpl() { }

        virtual void OnBeginDragging() { }
        virtual void OnDragging() { }
    };

    struct TranslateImpl : ModeImpl {
        EntityEditor*           entityEditor;
        std::vector<M::Vector3> oldEntPos;

        TranslateImpl(EntityEditor* entityEditor);

        void OnBeginDragging() override;
        void OnDragging() override;
    };

    struct ScaleImpl : ModeImpl {
        EntityEditor*               entityEditor;
        leda::node_map<M::Vector3>  oldVertPosF;

        ScaleImpl(EntityEditor* entityEditor);

        void OnBeginDragging() override;
        void OnDragging() override;
    };

    GEN::Pointer<ModeImpl>  _impl[2];
    ModeImpl*               _curImpl;

    int _allowedModeFlags;
    int _mode;

    void UpdateGizmo();

    bool DoPicking(const EV::MouseEvent& event, bool simulate);
protected:
    void OnBeginDragging() override;
    void OnDragging() override;
    bool OnMouseEvent(const EV::MouseEvent& event, bool simulate) override;
    bool OnKeyEvent(const EV::KeyEvent& event, bool simulate) override;
public:
    EntityEditor();

    void SetAllowedModeFlags(int flags);
    void SetMode(int mode);

    void Open();
    void Close();
};

} // namespace NB