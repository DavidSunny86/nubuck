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
#include <Nubuck\events\events.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world.h>
#include <world\entity.h>
#include <UI\outliner\outliner_fwd.h>

namespace UI {

struct OutlinerView : public QWidget, public EV::EventHandler<> { 
    virtual ~OutlinerView() { }
    DECL_HANDLE_EVENTS(OutlinerView);
};

class Outliner : public QWidget, public EV::EventHandler<> {
public:
private:
    typedef EV::EventHandler<> baseHandler_t;

    struct LinkedItem {
        LinkedItem          *prev, *next;

        QString             name;
        bool                isVisible;

        QTreeWidgetItem*    headerIt;
        QTreeWidgetItem* 	contentIt;

        QPushButton*        header;

        W::Entity*          entity; // used to create view
        OutlinerView*       view;

        LinkedItem() 
            : prev(NULL)
            , next(NULL)
            , isVisible(false)
            , headerIt(NULL)
            , contentIt(NULL)
            , header(NULL)
            , entity(NULL)
            , view(NULL)
        { }
	};

    SYS::SpinLock   _itemsMtx;
    LinkedItem*     _items;
    QTreeWidget*    _treeWidget;

    void CreateView(LinkedItem* item);

    void Event_CreateView(const EV::Event& event);
    void Event_Hide(const EV::Event& event);
    void Event_SetName(const EV::Event& event);
    void Event_Delete(const EV::Event& event);
public:
    typedef LinkedItem* itemHandle_t;

    Outliner(QWidget* parent = NULL);

    void HandleEvents();

    itemHandle_t    AddItem(const QString& name, W::Entity* entity);
    void            DeleteItem(itemHandle_t item);
    void            HideItem(itemHandle_t item);
    void            SetItemName(itemHandle_t item, const QString& name);

    void            SendToView(const itemHandle_t item, const EV::Event&);

}; // class Outliner

} // namespace UI

BEGIN_EVENT_DEF(Outliner_CreateView)
    UI::Outliner::itemHandle_t item;
END_EVENT_DEF

BEGIN_EVENT_DEF(Outliner_Hide)
    UI::Outliner::itemHandle_t item;
END_EVENT_DEF

BEGIN_EVENT_DEF(Outliner_SetName)
    UI::Outliner::itemHandle_t  item;
    QString*                    name;
END_EVENT_DEF

BEGIN_EVENT_DEF(Outliner_Delete)
    UI::Outliner::itemHandle_t item;
END_EVENT_DEF