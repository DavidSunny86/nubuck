#pragma once

#include <Nubuck\nb_common.h>
#include <Nubuck\editors\vertex_editor.h>

namespace OP {

class VDH_Operator : public Operator {
private:
    NB::Mesh _verticesMesh;
    NB::Mesh _delaunayMesh;
    NB::Mesh _voronoiMesh;
    NB::Mesh _hullMesh;

    leda::node_array<R::Color> _vertexColors;

    NB::VertexEditor _vertexEditor;

    void ApplyVoronoiColors();

    void Update();
public:
    VDH_Operator();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override;

    void OnMouse(const EV::MouseEvent& event) override;
};

} // namespace OP