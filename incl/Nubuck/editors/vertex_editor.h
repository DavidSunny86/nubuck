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
    leda::node_map<bool>                _selection;

    R::Color                            _col_unselected;
    R::Color                            _col_selected;

    leda::node_map<R::Color>            _oldColors;

    leda::node_map<M::Vector3>          _oldVertPosF;
    leda::node_map<leda::d3_rat_point>  _oldVertPosR;
    M::Vector3                          _center; // of selection

    W::ENT_Geometry*                    _subject;
    
    void UpdateGizmo();

    bool DoPicking(const EV::MouseEvent& event);
protected:
    void OnBeginDragging() override;
    void OnDragging() override;
    bool OnMouseEvent(const EV::MouseEvent& event) override;
public:
    VertexEditor();

    void Open(W::ENT_Geometry* geom);
    void Close();
};

} // namespace NB