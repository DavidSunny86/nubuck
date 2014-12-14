#include <QScrollBar>
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
    if(_entity) {
        if(Qt::ShiftModifier & event->modifiers()) {
            W::world.Select_Add(_entity);
        } else {
            W::world.Select_New(_entity);
        }
    }
}

void FocusCameraButton::mousePressEvent(QMouseEvent* event) {
    if(_entity) {
        ArcballCamera& camera = W::world.GetCamera();
        const M::Vector3& target = _entity->GetPosition(); // TODO: use center of bbox instead
        const float transitionDur = 0.25f; // same as rotation
        camera.TranslateTo(target, transitionDur);
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

void Outliner::Event_CreateView(const EV::Arg<outlinerItem_t>& event) {
    LinkedItem* item = event.value;

    // build header widget
    item->header.selection = new SelectEntityButton(item->entity);
    FocusCameraButton* focusButton = new FocusCameraButton(item->entity);

    item->header.name = new NameEntityButton(item->entity, QString("Polyhedron %1").arg(0));
    item->header.name->setObjectName("objectName");
    item->header.name->setIcon(QIcon(":/ui/Images/edit.svg"));
    item->header.name->setLayoutDirection(Qt::RightToLeft); // places icon on right-hand side

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(11, 0, 0, 0); // tightly packed
    headerLayout->addWidget(item->header.selection);
    headerLayout->addWidget(focusButton);
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

void Outliner::Event_Hide(const EV::Arg<outlinerItem_t>& event) {
    SYS::ScopedLock lock(_itemsMtx);
    event.value->headerIt->setHidden(true);
}

void Outliner::Event_SetName(const EV::Args2<outlinerItem_t, QString*>& event) {
    LinkedItem* item = event.value0;
    QString* name = event.value1;
    item->header.name->setText(*name);
    delete name;
}

void Outliner::Event_Delete(const EV::Arg<outlinerItem_t>& event) {
    SYS::ScopedLock lock(_itemsMtx);

    LinkedItem* item = event.value;

	if(item->next) item->next->prev = item->prev;
	if(item->prev) item->prev->next = item->next;
	if(_items == item) _items = item->next;
    if(item->headerIt) delete item->headerIt;
    delete item;
}

void Outliner::Event_SelectionChanged(const EV::Event&) {
    LinkedItem* it = _items;
    while(it) {
        bool isSelected = it->entity->IsSelected();
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

    baseHandler_t::Send(ev_outl_createView.Tag(item));

    return item;
}

// NOTE that this method must be called from main/ui thread
// for the event handler is called directly
void Outliner::DeleteItem(itemHandle_t item) {
    Event_Delete(EV::Arg<LinkedItem*>(item));
}

void Outliner::HideItem(itemHandle_t item) {
    baseHandler_t::Send(ev_outl_hide.Tag(item));
}

void Outliner::SetItemName(itemHandle_t item, const QString& name) {
    EV::Args2<LinkedItem*, QString*> event(item, new QString(name));
    baseHandler_t::Send(ev_outl_setName.Tag(event));
}

void Outliner::SendToView(itemHandle_t item, const EV::Event& event) {
    if(item->view) item->view->Send(event);
}

Outliner::Outliner(QWidget* parent) : QWidget(parent), _items(NULL) {
    _treeWidget = new QTreeWidget(this);
    _treeWidget->setHeaderHidden(true);

    const QString transparentBg = QString("background-color: rgba(0, 0, 0, 0);");
    _treeWidget->verticalScrollBar()->parentWidget()->setStyleSheet(transparentBg);
    _treeWidget->horizontalScrollBar()->parentWidget()->setStyleSheet(transparentBg);

    // mitigate ugly gradient in corner
    _treeWidget->setCornerWidget(new QWidget);
    _treeWidget->cornerWidget()->setStyleSheet("border: 1px solid #1E1E1E; background-color: rgba(0, 0, 0, 0);");

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_treeWidget);
    setLayout(layout);

    AddEventHandler(ev_outl_createView, this, &Outliner::Event_CreateView);
    AddEventHandler(ev_outl_hide, this, &Outliner::Event_Hide);
    AddEventHandler(ev_outl_setName, this, &Outliner::Event_SetName);
    AddEventHandler(ev_outl_delete, this, &Outliner::Event_Delete);
    AddEventHandler(ev_w_selectionChanged, this, &Outliner::Event_SelectionChanged);
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