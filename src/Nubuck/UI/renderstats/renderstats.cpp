#include <Nubuck\common\common.h>
#include "renderstats.h"

namespace UI {

void RenderStats::OnContextChanged(int idx) {
    _textEdit->clear();
    if(0 <= idx) {
        std::string context = _contexts->currentText().toStdString();
        COM_assert(!context.empty());
        COM_assert(_stats.end() != _stats.find(context));
        _textEdit->appendPlainText(QString::fromStdString(_stats[context]));
    }
}

RenderStats::RenderStats(QWidget* parent) : QWidget(parent, Qt::WindowStaysOnTopHint) {
    setWindowTitle("Render Statistics");

    _contexts = new QComboBox();
    connect(_contexts, SIGNAL(currentIndexChanged(int)),
        this, SLOT(OnContextChanged(int)));

    _textEdit = new QPlainTextEdit;

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_contexts);
    layout->addWidget(_textEdit);
    setLayout(layout);
}

RenderStats* RenderStats::Instance() {
    static RenderStats* theInstance = NULL;
    if(!theInstance) theInstance = new RenderStats;
    return theInstance;
}

void RenderStats::Update(const std::string& context, const std::string& stats) {
    if(context.empty()) return;

    const bool rebuildComboBox = _stats.end() == _stats.find(context);

    _stats[context] = stats;

    if(rebuildComboBox) {
        _contexts->blockSignals(true);
        _contexts->clear();
        std::map<std::string, std::string>::const_iterator it;
        for(it = _stats.begin(); _stats.end() != it; ++it) {
            _contexts->addItem(QString::fromStdString(it->first));
        }
        _contexts->blockSignals(false);
    }

    OnContextChanged(_contexts->currentIndex());
}

} // namespace UI