#pragma once

#include <vector>

#include <QGridLayout>
#include <QTreeWidget>

#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QVBoxLayout>

#include <UI\colorbutton\colorbutton.h>
#include <events\events.h>
#include <world\world.h>

namespace UI {

struct PolyhedronOutline : QObject {
    Q_OBJECT
public slots:
    void OnEdgeColorChanged(float r, float g, float b);
    void OnEdgeRadiusChanged(double value);
    void OnHullAlphaChanged(int value);
public:
    unsigned            entId;
    QDoubleSpinBox*     sbEdgeRadius;
    ColorButton*        btnEdgeColor;
};

class Outliner : public QWidget, public EV::EventHandler<Outliner> {
    DECLARE_EVENT_HANDLER(Outliner);
private:
    typedef W::World::EntityType EntityType;


    struct EntityOutline {
        EntityType          type;
        unsigned    		entId;
        QPushButton*        headerWidget;
        QWidget*            widget;
        QGridLayout*        layout;
        PolyhedronOutline*  polyhedron;
    };

    void EntityOutline_Init(EntityOutline& outln);
    void PolyhedronOutline_Init(EntityOutline& outln);

    std::vector<EntityOutline>  _outlines;
    QTreeWidget*                _treeWidget;

    EntityOutline* FindByEntityID(unsigned entId);

#pragma region EventHandlers
    void Event_SpawnPolyhedron(const EV::Event& event);
    void Event_SetName(const EV::Event& event);
    void Event_EntityInfo(const EV::Event& event);
// region EventHandlers
#pragma endregion
public:
    Outliner(QWidget* parent = NULL);

    static Outliner* Instance(void);

    void Update(void) { HandleEvents(); }
}; // class Outliner

} // namespace UI