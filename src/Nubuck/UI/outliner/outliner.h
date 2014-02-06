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

namespace UI {

class Outliner : public QWidget {
public:
    struct Item {
        QPushButton*    header;
        QWidget*        content;
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

    itemHandle_t AddItem(const QString& name, QWidget* content);
    void RemoveItem(itemHandle_t item);

	Item& GetItem(itemHandle_t item) { return *item; }
    void UpdateItem(itemHandle_t item) {
    _treeWidget->setItemWidget(item->contentIt, 0, item->content);
	}

    static Outliner* Instance(void);
}; // class Outliner

} // namespace UI