#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\nubuck_api.h>
#include <Nubuck\editors\editor.h>
#include <Nubuck\renderer\color\color.h>

namespace W {
    class ENT_Geometry;
    class ENT_TransformGizmo;
};

namespace NB {

class NUBUCK_API VertexEditor : public Editor {
private:
    struct ModeImpl {
        virtual ~ModeImpl() { }
        virtual void OnDragging() = 0;
    };

    struct TranslateImpl : ModeImpl {
        VertexEditor* vertexEditor;
        TranslateImpl(VertexEditor* vertexEditor);
        void OnDragging() override;
    };

    struct ScaleImpl : ModeImpl {
        VertexEditor* vertexEditor;
        ScaleImpl(VertexEditor* vertexEditor);
        void OnDragging() override;
    };

    GEN::Pointer<ModeImpl>  _impl[2];
    ModeImpl*               _curImpl;

    int                                 _allowedModeFlags;
    int                                 _mode;

    bool                                _modifyGlobalSelection;

    leda::node_map<bool>                _selection;
    unsigned                            _numSelected;

    void ClearSelection();
    void Select(leda::node v);
    bool IsSelected(leda::node v) { return _selection[v]; }

    R::Color                            _col_unselected;
    R::Color                            _col_selected;

    leda::node_map<R::Color>            _oldVertColors;
    leda::edge_map<R::Color>            _oldEdgeColors;

    leda::node_map<M::Vector3>          _oldVertPosF;
    leda::node_map<leda::d3_rat_point>  _oldVertPosR;
    M::Vector3                          _center; // of selection

    W::ENT_Geometry*                    _subject;

    // used by impl classes
    W::ENT_Geometry*    GetSubject() { return _subject; }
    M::Vector3          GetOldVertexPosition(leda::node v) const { return _oldVertPosF[v]; }
    const M::Vector3&   GetCenterOfSelection() const { return _center; }

    bool DoPicking(const EV::MouseEvent& event, bool simulate);

    leda::node SearchSelectedVertex(leda::node v);
protected:
    void OnBeginDragging() override;
    void OnDragging() override;
    bool OnMouseEvent(const EV::MouseEvent& event, bool simulate) override;
    bool OnKeyEvent(const EV::KeyEvent& event, bool simulate) override;
public:
    VertexEditor();
    
    void        UpdateGizmo();

    void        SetAllowedModeFlags(int flags);
    void        SetMode(int mode);

    void        SetModifyGlobalSelection(bool modify);

    void        Open(W::ENT_Geometry* geom);
    void        Close();

    leda::node  FirstSelectedVertex();
    leda::node  NextSelectedVertex(leda::node v);
};

} // namespace NB