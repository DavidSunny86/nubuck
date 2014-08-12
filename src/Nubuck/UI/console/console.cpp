#include <common\commands.h>
#include <UI\plaintextdevice.h>
#include "console.h"

namespace UI {

void Console::OnExec() {
    QByteArray cmd = _ui.edtInput->text().toAscii();
    COM::CMD::ExecCommand(_textStream, cmd.data());
    _textStream.flush();

    _ui.edtInput->clear();
}

Console::Console(QWidget* parent) : QWidget(parent) {
    _ui.setupUi(this);

    _textDevice = GEN::MakePtr(new PlainTextDevice(_ui.edtOutput));
    _textStream.setDevice(_textDevice.Raw());
}

} // namespace UI