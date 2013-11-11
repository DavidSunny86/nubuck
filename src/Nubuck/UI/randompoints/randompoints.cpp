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

static void RandomPointsInHemiball(int n, int R, leda::list<point_t>& L) {
    leda::random_d3_rat_points_in_ball(n, R, L);
    leda::list_item it;
    forall_items(it, L) {
        point_t& p = L[it];
        scalar_t z = p.zcoord();
        if(0 > z) p = p.translate(0, 0, -2 * z);
    }
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

    _pointGens[HEMISPHERE] = RandomPointsInHemiball;
    cbShapes->insertItem(HEMISPHERE, "Hemisphere");

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
