#pragma once

#include <map>
#include <string>

#include <QtGui>

namespace UI {

class RenderStats : public QWidget {
    Q_OBJECT
private:
    std::map<std::string, std::string> _stats; // maps context name to stats

    QComboBox*      _contexts;
    QPlainTextEdit* _textEdit;
private slots:
    void OnContextChanged(int idx);
public:
    RenderStats(QWidget* parent = NULL);

    static RenderStats* Instance();

    void Update(const std::string& context, const std::string& stats);
};

} // namespace UI