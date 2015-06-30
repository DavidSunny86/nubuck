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
        Action_SelectEntity,
        Action_SelectEntity_Add,
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
        EntityEditor* entityEditor;

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

        M::Vector3  initialPos; // position of entity at time of selection

        // used by translate impl
        M::Vector3  oldPos;

        // used by scale impl
        leda::node_map<M::Vector3> oldVertPosF;

        EntityData() : isSelected(false), nextSelected(NULL) { }
    };

    GEN::Pointer<ScaleImpl> _scaleImpl;

    GEN::Pointer<ModeImpl>  _impl[2];
    ModeImpl*               _curImpl;

    std::vector<EntityData> _entData; // indexed with entity IDs
    W::Entity*              _selected;

    int                     _allowedModeFlags;
    int                     _mode;

    int                     _lastAction;

    bool IsSelected(const W::Entity* ent) const;

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

    void Open();
    void Close();

    void ClearSelection();
    void SelectEntity_Add(W::Entity* ent);
    void SelectEntity_New(W::Entity* ent);

    void CopyGlobalSelection();

    void SetTranslationVector(const M::Vector3& v);

    void UpdateBoundingBoxes();

    M::Vector3          GlobalCenterOfSelection();
    W::Entity*          FirstSelectedEntity();
    const W::Entity*    FirstSelectedEntity() const;
    W::Entity*          NextSelectedEntity(const W::Entity* ent);

    Mode        GetMode() const;

    // return last action that caused Handle*Event() to return true
    Action      GetAction() const;

    M::Vector3  GetTranslationVector() const;
    M::Vector3  GetScalingVector() const;
};

void CopyGlobalSelection(EntityEditor& editor);

} // namespace NB