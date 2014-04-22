#pragma once

#include <QIODevice>
#include <QPlainTextEdit>

namespace UI {

class PlainTextDevice : public QIODevice {
private:
    QPlainTextEdit* _subject;
protected:
    qint64 readData(char* data, qint64 maxSize) override;
    qint64 writeData(const char* data, qint64 maxSize) override;
public:
    explicit PlainTextDevice(QPlainTextEdit* subject);
};

} // namespace UI