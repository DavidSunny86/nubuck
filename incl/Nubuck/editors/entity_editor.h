#pragma once

#include <vector>
#include <LEDA\graph\node_map.h>
#include <Nubuck\editors\editor.h>

namespace NB {

/*
TODO:
- cache bounding boxes
*/

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

    struct BBox {
        W::ENT_Geometry*    geom;
        leda::node          verts[8];

        BBox() : geom(NULL) { }

        void Init();
        void Update(W::Entity* ent);
        void Destroy();
    };

    struct EntityData {
        bool        isSelected;
        BBox        bbox;
        W::Entity*  nextSelected;

        EntityData() : isSelected(false), nextSelected(NULL) { }
    };

    std::vector<EntityData> _entData; // indexed with entity IDs
    W::Entity*              _selected;

    int _allowedModeFlags;
    int _mode;

    bool _modifyGlobalSelection;

    bool IsSelected(const W::Entity* ent) const;

    void ClearSelection();
    void SelectEntity_Add(W::Entity* ent);

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

    void SetModifyGlobalSelection(bool modify);

    void Open();
    void Close();

    void UpdateBoundingBoxes();

    void CopyGlobalSelection();

    M::Vector3  GlobalCenterOfSelection();
    W::Entity*  FirstSelectedEntity();
    W::Entity*  NextSelectedEntity(const W::Entity* ent);
};

} // namespace NB