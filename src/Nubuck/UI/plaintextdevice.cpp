#include "plaintextdevice.h"

namespace UI {

qint64 PlainTextDevice::readData(char*, qint64) { return -1; }

qint64 PlainTextDevice::writeData(const char* data, qint64 maxSize) {
    QString text = QString(data);
    _subject->moveCursor(QTextCursor::End);
    _subject->insertPlainText(text);
    _subject->moveCursor(QTextCursor::End);
    return text.length();
}

PlainTextDevice::PlainTextDevice(QPlainTextEdit* subject) : _subject(subject) {
    open(QIODevice::WriteOnly);
}

} // namespace UI