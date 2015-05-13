#pragma once

#include <QAction>
#include <QEvent>

namespace UI {

class MenuItem : public QAction {
protected:
    bool event(QEvent* ev) override;
public:
    MenuItem(QObject* parent = NULL);
    MenuItem(const QString& text, QObject* parent = NULL);
};

} // namespace UI