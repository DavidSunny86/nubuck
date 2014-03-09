#include <UI\window_events.h>
#include <UI\logwidget\logwidget.h>
#include "outliner.h"

namespace UI {

void Outliner::Event_CreateView(const EV::Event& event) {
    const EV::Params_Outliner_CreateView& args = EV::def_Outliner_CreateView.GetArgs(event);
    LinkedItem* item = args.item;

    QTreeWidgetItem* headerIt = new QTreeWidgetItem;
    _treeWidget->insertTopLevelItem(0, headerIt);
    QPushButton* button = new QPushButton(QString("Polyhedron %1").arg(0));
    button->setStyleSheet("text-align: left");
    _treeWidget->setItemWidget(headerIt, 0, button);
    item->headerIt = headerIt;
    item->header = button;

    QTreeWidgetItem* contentIt = new QTreeWidgetItem;
    headerIt->addChild(contentIt);
    item->view = item->entity->CreateOutlinerView();
    _treeWidget->setItemWidget(contentIt, 0, item->view);
    item->contentIt = contentIt;

    item->isVisible = true;
}

void Outliner::Event_Hide(const EV::Event& event) {
    SYS::ScopedLock lock(_itemsMtx);
    const EV::Params_Outliner_Hide& args = EV::def_Outliner_Hide.GetArgs(event);
    args.item->headerIt->setHidden(true);
}

void Outliner::Event_Delete(const EV::Event& event) {
    SYS::ScopedLock lock(_itemsMtx);

    const EV::Params_Outliner_Delete& args = EV::def_Outliner_Delete.GetArgs(event);
    LinkedItem* item = args.item;

	if(item->next) item->next->prev = item->prev;
	if(item->prev) item->prev->next = item->next;
	if(_items == item) _items = item->next;
    if(item->headerIt) delete item->headerIt;
    delete item;
}

Outliner::itemHandle_t Outliner::AddItem(const QString& name, W::Entity* entity) {
    SYS::ScopedLock lock(_itemsMtx);

    LinkedItem* item = new LinkedItem();

    item->name = name;
    item->entity = entity;

	item->prev = NULL;
	item->next = _items;
	if(item->next) item->next->prev = item;
    _items = item;

    EV::Params_Outliner_CreateView args = { item };
    baseHandler_t::Send(EV::def_Outliner_CreateView.Create(args));

    return item;
}

void Outliner::DeleteItem(itemHandle_t item) {
    EV::Params_Outliner_Delete args = { item };
    baseHandler_t::Send(EV::def_Outliner_Delete.Create(args));
}

void Outliner::HideItem(itemHandle_t item) {
    EV::Params_Outliner_Hide args = { item };
    baseHandler_t::Send(EV::def_Outliner_Hide.Create(args));
}

void Outliner::SendToView(itemHandle_t item, const EV::Event& event) {
    if(item->view) item->view->Send(event);
}

Outliner::Outliner(QWidget* parent) : QWidget(parent), _items(NULL) {
    _treeWidget = new QTreeWidget(this);
    _treeWidget->setHeaderHidden(true);
    
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_treeWidget);
    setLayout(layout);

    AddEventHandler(EV::def_Outliner_CreateView, this, &Outliner::Event_CreateView);
    AddEventHandler(EV::def_Outliner_Hide, this, &Outliner::Event_Hide);
    AddEventHandler(EV::def_Outliner_Delete, this, &Outliner::Event_Delete);
}

Outliner* Outliner::Instance(void) {
    static Outliner* instance = NULL;
    if(!instance) instance = new Outliner();
    return instance;
}

void Outliner::HandleEvents() {
    SYS::ScopedLock lock(_itemsMtx);
    _EV_HandleEvents(this, "Outliner");
    LinkedItem* it = _items;
    while(it) {
        if(it->view) it->view->HandleEvents();
        it = it->next;
    }
}

} // namespace UI