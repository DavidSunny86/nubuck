#include <UI\window_events.h>
#include <UI\logwidget\logwidget.h>
#include "outliner.h"

namespace UI {

/*
void Outliner::PolyhedronOutline_Init(EntityOutline& outln) {
    outln.polyhedron = new PolyhedronOutline;

    QLabel* lblEdgeRadius = new QLabel("edge radius:");
    QDoubleSpinBox* sbEdgeRadius = new QDoubleSpinBox;
    sbEdgeRadius->setMinimum(0.05f);
    sbEdgeRadius->setMaximum(5.00f);
    sbEdgeRadius->setSingleStep(0.1f);

    QLabel* lblEdgeColor = new QLabel("edge color:");
    ColorButton* btnEdgeColor = new ColorButton;

    QLabel* lblHullAlpha = new QLabel("hull alpha:");
    QSlider* sldHullAlpha = new QSlider(Qt::Horizontal);
    sldHullAlpha->setTracking(true);
    sldHullAlpha->setMinimum(0);
    sldHullAlpha->setMaximum(100);
    sldHullAlpha->setValue(100);

    outln.layout->addWidget(lblEdgeRadius, 0, 0, 1, 1);
    outln.layout->addWidget(sbEdgeRadius, 0, 1, 1, 1);

    outln.layout->addWidget(lblEdgeColor, 1, 0, 1, 1);
    outln.layout->addWidget(btnEdgeColor, 1, 1, 1, 1);

    outln.layout->addWidget(lblHullAlpha, 2, 0, 1, 1);
    outln.layout->addWidget(sldHullAlpha, 2, 1, 1, 1);

    outln.polyhedron->entId = outln.entId;
    outln.polyhedron->sbEdgeRadius = sbEdgeRadius;
    outln.polyhedron->btnEdgeColor = btnEdgeColor;

    connect(btnEdgeColor, SIGNAL(SigColorChanged(float, float, float)), outln.polyhedron, SLOT(OnEdgeColorChanged(float, float, float)));
    connect(sbEdgeRadius, SIGNAL(valueChanged(double)), outln.polyhedron, SLOT(OnEdgeRadiusChanged(double)));
    connect(sldHullAlpha, SIGNAL(valueChanged(int)), outln.polyhedron, SLOT(OnHullAlphaChanged(int)));
}
*/

Outliner::View::View() : _isDead(false) {
    AddEventHandler(EV::def_Outliner_DeleteOutline, this, &Outliner::View::Event_DeleteOutline);
}

void Outliner::HandleEvents() {
    LinkedItem* it = _items;
    while(it) {
		it->view->HandleEvents();
		it = it->next;
	}
}

Outliner::itemHandle_t Outliner::AddItem(const QString& name, const GEN::Pointer<View>& view) {
    LinkedItem* item = new LinkedItem();

    QTreeWidgetItem* headerIt = new QTreeWidgetItem;
    _treeWidget->insertTopLevelItem(0, headerIt);
    QPushButton* button = new QPushButton(QString("Polyhedron %1").arg(0));
    button->setStyleSheet("text-align: left");
    _treeWidget->setItemWidget(headerIt, 0, button);
	item->headerIt = headerIt;
    item->header = button;

    QTreeWidgetItem* contentIt = new QTreeWidgetItem;
    headerIt->addChild(contentIt);
    item->view = view;
    _treeWidget->setItemWidget(contentIt, 0, item->view.Raw());
	item->contentIt = contentIt;


	item->prev = NULL;
	item->next = _items;
	if(item->next) item->next->prev = item;
    _items = item;

    return item;
}

void Outliner::RemoveItem(itemHandle_t item) {
	if(item->next) item->next->prev = item->prev;
	if(item->prev) item->prev->next = item->next;
	if(_items == item) _items = item->next;
	delete item->headerIt;
    delete item;
}

Outliner::Outliner(QWidget* parent) : QWidget(parent), _items(NULL) {
    _treeWidget = new QTreeWidget(this);
    _treeWidget->setHeaderHidden(true);
    
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_treeWidget);
    setLayout(layout);
}

Outliner* Outliner::Instance(void) {
    static Outliner* instance = NULL;
    if(!instance) instance = new Outliner();
    return instance;
}

} // namespace UI