#include <QMouseEvent>
#include <QInputDialog>
#include <UI\window_events.h>
#include <UI\logwidget\logwidget.h>
#include <world\world_events.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <world\world.h>
#include "outliner.h"

namespace UI {

void SelectEntityButton::mousePressEvent(QMouseEvent* event) {
    if(_entity && W::EntityType::ENT_GEOMETRY == _entity->GetType()) {
        W::ENT_Geometry* geom = (W::ENT_Geometry*)_entity;
        if(Qt::ShiftModifier & event->modifiers()) {
            W::world.GetSelection()->Add(geom);
        } else {
            W::world.GetSelection()->Set(geom);
        }
    }
}

void NameEntityButton::mousePressEvent(QMouseEvent*) {
    bool ok;
    QString oldName = QString::fromStdString(_entity->GetName());
    QString newName = QInputDialog::getText(this, "Choose Entity Name", "Entity name:", QLineEdit::Normal, oldName, &ok);
    if(ok) {
        _entity->SetName(newName.toStdString());
    }
}

void Outliner::Event_CreateView(const EV::Event& event) {
    const EV::Params_Outliner_CreateView& args = EV::def_Outliner_CreateView.GetArgs(event);
    LinkedItem* item = args.item;

    // build header widget
    item->header.selection = new SelectEntityButton(item->entity);

    item->header.name = new NameEntityButton(item->entity, QString("Polyhedron %1").arg(0));
    item->header.name->setObjectName("objectName");
    item->header.name->setIcon(QIcon(":/ui/Images/edit.svg"));
    item->header.name->setLayoutDirection(Qt::RightToLeft); // places icon on right-hand side

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(11, 0, 0, 0); // tightly packed
    headerLayout->addWidget(item->header.selection);
    headerLayout->addWidget(item->header.name);
    QWidget* headerWidget = new QWidget;
    headerWidget->setObjectName("outlinerHeader");
    headerWidget->setLayout(headerLayout);

    QTreeWidgetItem* headerIt = new QTreeWidgetItem;
    _treeWidget->insertTopLevelItem(0, headerIt);
    _treeWidget->setItemWidget(headerIt, 0, headerWidget);
    item->headerIt = headerIt;

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

void Outliner::Event_SetName(const EV::Event& event) {
    const EV::Params_Outliner_SetName& args = EV::def_Outliner_SetName.GetArgs(event);
    args.item->header.name->setText(*args.name);
    delete args.name;
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

void Outliner::Event_SelectionChanged(const EV::Event&) {
    LinkedItem* it = _items;
    while(it) {
        bool isSelected = false;
        if(W::EntityType::ENT_GEOMETRY == it->entity->GetType()) {
            W::ENT_Geometry* geom = (W::ENT_Geometry*)it->entity;
            isSelected = geom->IsSelected();
        }
        if(it->header.selection) it->header.selection->setChecked(isSelected);
        it = it->next;
    }
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

// NOTE that this method must be called from main/ui thread
// for the event handler is called directly
void Outliner::DeleteItem(itemHandle_t item) {
    EV::Params_Outliner_Delete args = { item };
    Event_Delete(EV::def_Outliner_Delete.Create(args));
}

void Outliner::HideItem(itemHandle_t item) {
    EV::Params_Outliner_Hide args = { item };
    baseHandler_t::Send(EV::def_Outliner_Hide.Create(args));
}

void Outliner::SetItemName(itemHandle_t item, const QString& name) {
    EV::Params_Outliner_SetName args = { item, new QString(name) };
    baseHandler_t::Send(EV::def_Outliner_SetName.Create(args));
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
    AddEventHandler(EV::def_Outliner_SetName, this, &Outliner::Event_SetName);
    AddEventHandler(EV::def_Outliner_Delete, this, &Outliner::Event_Delete);
    AddEventHandler(EV::def_SelectionChanged, this, &Outliner::Event_SelectionChanged);
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