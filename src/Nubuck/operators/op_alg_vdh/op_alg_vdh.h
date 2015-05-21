#pragma once

#include <Nubuck\nb_common.h>
#include <Nubuck\editors\vertex_editor.h>

namespace OP {

enum {
    ID_DELAUNAY,
    ID_VORONOI,
    ID_CHULL,
    ID_PARABOLOID
};

class VDH_Panel : public OperatorPanel {
private:
    NB::CheckBox _cbDelaunay;
    NB::CheckBox _cbVoronoi;
    NB::CheckBox _cbCHull;
    NB::CheckBox _cbParaboloid;
public:
    VDH_Panel();

    void Invoke() override;
};

class VDH_Operator : public Operator {
private:
    NB::Mesh _verticesMesh;
    NB::Mesh _delaunayMesh;
    NB::Mesh _voronoiMesh;
    NB::Mesh _hullMesh;
    NB::Mesh _paraboloidMesh;

    leda::node_array<R::Color> _vertexColors;

    // maps subset of E(voronoiMesh) to V(verticesMesh)
    leda::edge_map<leda::node> _site;

    NB::VertexEditor _vertexEditor;

    void ApplyVoronoiColors();

    void Update();

    void Event_CheckBoxToggled(const EV::Arg<bool>& event);
public:
    VDH_Operator();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override;

    void OnMouse(const EV::MouseEvent& event) override;
};

} // namespace OP