#pragma once

#include <QDialog>

#include <LEDA\core\list.h>
#include <LEDA\core\random_source.h>

#include <common\types.h>
#include "ui_randompoints.h"

class RandomPoints : public QDialog, public Ui::RandomPoints {
    Q_OBJECT
private slots:
    void OnRandomSize(void);
    void OnRandomRadius(void);
private:
    enum {
        MIN_SIZE = 10,
        MAX_SIZE = 50000,

        MIN_RADIUS = 10,
        MAX_RADIUS = 5000
    };

    enum {
        SPHERE = 0,
        CUBE,
        PARABOLOID,

        NUM_SHAPES
    };

    typedef void (*pointGen_t)(int size, int radius, 
            leda::list<point_t>& L);

    pointGen_t _pointGens[NUM_SHAPES];

    leda::random_source _sizeSrc;
    leda::random_source _radiusSrc;
public:
    RandomPoints(QWidget* parent = NULL);

	leda::list<point_t> AsList(void) const;
	graph_t				AsGraph(void) const;
};
