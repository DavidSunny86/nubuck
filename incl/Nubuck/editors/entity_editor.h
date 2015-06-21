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
public:
    enum Mode {
        Mode_Translate = 0,
        Mode_Scale,
        Mode_Rotate
    };

    enum Action {
        Action_None = 0,
        Action_BeginDragging,
        Action_Dragging,
        Action_EndDragging,
        Action_PickEntity
    };
private:
    struct ModeImpl {
        virtual ~ModeImpl() { }

        virtual void OnEnter() { }
        virtual void OnBeginDragging() { }
        virtual void OnDragging() { }
        virtual void OnEndDragging() { }
    };

    struct TranslateImpl : ModeImpl {
        EntityEditor*           entityEditor;
        std::vector<M::Vector3> oldEntPos;

        TranslateImpl(EntityEditor* entityEditor);

        void OnBeginDragging() override;
        void OnDragging() override;
    };

    struct ScaleImpl : ModeImpl {
        EntityEditor*   entityEditor;
        M::Vector3      lastScale;
        M::Vector3      scale;

        ScaleImpl(EntityEditor* entityEditor);

        void OnEnter() override;
        void OnBeginDragging() override;
        void OnDragging() override;
        void OnEndDragging() override;
    };

    GEN::Pointer<ScaleImpl> _scaleImpl;

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

        // used by scale impl
        leda::node_map<M::Vector3> oldVertPosF;

        EntityData() : isSelected(false), nextSelected(NULL) { }
    };

    std::vector<EntityData> _entData; // indexed with entity IDs
    W::Entity*              _selected;

    int _allowedModeFlags;
    int _mode;

    bool _modifyGlobalSelection;

    int _lastAction;

    bool IsSelected(const W::Entity* ent) const;

    void ClearSelection();
    void SelectEntity_Add(W::Entity* ent);

    void UpdateGizmo();

    bool DoPicking(const EV::MouseEvent& event, bool simulate);
protected:
    EntityData& GetEntityData(W::Entity* ent);

    void OnBeginDragging() override;
    void OnDragging() override;
    void OnEndDragging() override;
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

    Mode        GetMode() const;

    // return last action that caused Handle*Event() to return true
    Action      GetAction() const;

    M::Vector3  GetScalingVector() const;
};

} // namespace NB