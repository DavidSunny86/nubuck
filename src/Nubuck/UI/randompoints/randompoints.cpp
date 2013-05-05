#include "randompoints.h"

void RandomPoints::OnRandomSize(void) {
    unsigned r;
    _sizeSrc >> r;
    sbSize->setValue(r);
}

void RandomPoints::OnRandomRadius(void) {
    unsigned r;
    _radiusSrc >> r;
    sbRadius->setValue(r);
}

RandomPoints::RandomPoints(QWidget* parent) : QDialog(parent) {
    setupUi(this);

    _sizeSrc.set_range(MIN_SIZE, MAX_SIZE);
    _radiusSrc.set_range(MIN_RADIUS, MAX_RADIUS);

    _pointGens[SPHERE] = leda::random_d3_rat_points_in_ball;
    cbShapes->insertItem(SPHERE, "Sphere");

    _pointGens[CUBE] = leda::random_d3_rat_points_in_cube;
    cbShapes->insertItem(CUBE, "Cube");

    _pointGens[PARABOLOID] = leda::random_d3_rat_points_on_paraboloid;
    cbShapes->insertItem(PARABOLOID, "Paraboloid");

    sbSize->setMinimum(MIN_SIZE);
    sbSize->setMaximum(MAX_SIZE);
    sbSize->setValue(MIN_SIZE);

    sbRadius->setMinimum(MIN_RADIUS);
    sbRadius->setMaximum(MAX_RADIUS);
    sbRadius->setValue(MIN_SIZE);

    connect(btnRandomSize, SIGNAL(clicked()),
            this, SLOT(OnRandomSize()));
    connect(btnRandomRadius, SIGNAL(clicked()),
            this, SLOT(OnRandomRadius()));
}

leda::list<point_t> RandomPoints::AsList(void) const {
	leda::list<point_t> L;
	_pointGens[cbShapes->currentIndex()](sbSize->value(), sbRadius->value(), L);
	return L;
}

graph_t RandomPoints::AsGraph(void) const {
	leda::list<point_t> L = AsList();
	graph_t G;

	point_t p;
	forall(p, L) G[G.new_node()] = p;

	return G;
}
