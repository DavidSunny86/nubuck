#pragma once

#include <vector>

#include <QGridLayout>
#include <QTreeWidget>

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
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

struct SelectEntityButton : public QToolButton {
private:
    W::Entity*  _entity;

    QIcon 	_icoUnselected;
    QIcon   _icoSelected;
    QIcon*  _icons[2];

    int     _idx;
protected:
    void mousePressEvent(QMouseEvent*);
public:
    SelectEntityButton(W::Entity* entity)
        : _entity(entity)
        , _icoUnselected(":/ui/Images/unselected_object.svg")
        , _icoSelected(":/ui/Images/selected_object.svg")
        , _idx(0)
    {
        _icons[0] = &_icoUnselected;
        _icons[1] = &_icoSelected;
        setIcon(*_icons[_idx]);

        setObjectName("selectObject");
    }

    void setChecked(bool checked) {
        _idx = 1 ? checked : 0;
        setIcon(*_icons[_idx]);
    }
};

struct FocusCameraButton : public QToolButton {
private:
    W::Entity* _entity;
protected:
    void mousePressEvent(QMouseEvent* event);
public:
    FocusCameraButton(W::Entity* entity) : _entity(entity){
        setIcon(QIcon(":/ui/Images/focus_camera.svg"));
        setObjectName("selectObject");
    }
};

class NameEntityButton : public QPushButton {
private:
    W::Entity* _entity;
protected:
    void mousePressEvent(QMouseEvent*) override;
public:
    NameEntityButton(W::Entity* entity, const QString& name)
        : _entity(entity)
    { }
};

class Outliner : public QWidget, public EV::EventHandler<> {
public:
private:
    typedef EV::EventHandler<> baseHandler_t;

    struct Header {
        SelectEntityButton* selection;
        NameEntityButton*   name;
    };

    struct LinkedItem {
    public:
        LinkedItem          *prev, *next;

        QString             name;
        bool                isVisible;

        QTreeWidgetItem*    headerIt;
        QTreeWidgetItem* 	contentIt;

        Header              header;

        W::Entity*          entity; // used to create view
        OutlinerView*       view;

        LinkedItem()
            : prev(NULL)
            , next(NULL)
            , isVisible(false)
            , headerIt(NULL)
            , contentIt(NULL)
            , header()
            , entity(NULL)
            , view(NULL)
        { }
	};

    SYS::SpinLock   _itemsMtx;
    LinkedItem*     _items;
    QTreeWidget*    _treeWidget;

    void CreateView(LinkedItem* item);

    void Event_CreateView(const EV::Arg<LinkedItem*>& event);
    void Event_Hide(const EV::Arg<LinkedItem*>& event);
    void Event_SetName(const EV::Args2<LinkedItem*, QString*>& event);
    void Event_Delete(const EV::Arg<LinkedItem*>& event);
    void Event_SelectionChanged(const EV::Event& event);
public:
    typedef LinkedItem* itemHandle_t;

    Outliner(QWidget* parent = NULL);

    void HandleEvents();

    itemHandle_t    AddItem(const QString& name, W::Entity* entity);
    void            DeleteItem(itemHandle_t item);
    void            HideItem(itemHandle_t item);
    void            SetItemName(itemHandle_t item, const QString& name);

    void            SendToView(const itemHandle_t item, const EV::EventDef& def, const EV::Event&);

}; // class Outliner

} // namespace UI

typedef UI::Outliner::itemHandle_t outlinerItem_t;

extern EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_createView;
extern EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_hide;
extern EV::ConcreteEventDef<EV::Args2<outlinerItem_t, QString*> >   ev_outl_setName;
extern EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_delete;