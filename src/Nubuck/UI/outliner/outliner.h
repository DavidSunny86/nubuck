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
#include <world\entity.h>

BEGIN_EVENT_DEF(Outliner_DeleteOutline)
END_EVENT_DEF

namespace UI {

class Outliner : public QWidget {
public:
    struct View : public QWidget, public EV::EventHandler<> {
    private:
        bool    _isDead;

        void Event_DeleteOutline(const EV::Event& event) { _isDead = true; }
    public:
        View();
        virtual ~View() { }

        bool IsDead() const { return _isDead; }

        DECL_HANDLE_EVENTS(View);

        virtual void InitUI() = 0;
    };

    struct Item {
        QPushButton*        header;
        GEN::Pointer<View>  view; 
	};
private:
    struct LinkedItem : Item {
        LinkedItem *prev, *next;
        QTreeWidgetItem* headerIt;
        QTreeWidgetItem* contentIt;
	};

    LinkedItem*     _items;
    QTreeWidget*    _treeWidget;
public:
    typedef LinkedItem* itemHandle_t;

    Outliner(QWidget* parent = NULL);

    void HandleEvents();

    itemHandle_t AddItem(const QString& name, const GEN::Pointer<View>& view);
    void RemoveItem(itemHandle_t item);

	Item& GetItem(itemHandle_t item) { return *item; }

    void UpdateItem(itemHandle_t item) {
        _treeWidget->setItemWidget(item->contentIt, 0, item->view.Raw());
	}

    static Outliner* Instance(void);
}; // class Outliner

} // namespace UI