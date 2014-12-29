#include "logwidget.h"

namespace UI {

    void LogWidget::OnSetEnabled(bool enabled) {
        _enabled = enabled;
    }

    enum BlockType {
        BT_END = 0,
        BT_SYSTEM,
        BT_OPERATOR
    };

    LogWidget::LogWidget(QWidget* parent) : QWidget(parent), _enabled(true), _blockCnt(0) {
        _ui.setupUi(this);

        QTextCursor textCursor = QTextCursor(_ui.textBrowser->document());
        _defaultFgBrush = textCursor.blockCharFormat().foreground();

        _buffer.resize(4096);
        _buffer[0] = BT_END;
        _bufferCur = &_buffer[0];
    }

    void LogWidget::printf(int blockType, const char* format, va_list args) {
        const int BUFFER_SIZE = 2048;
        static char buffer[BUFFER_SIZE];
        if(_enabled) {
            memset(buffer, 0, sizeof(buffer));
            if(0 > vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args)) {
                strcpy(&buffer[BUFFER_SIZE - 5], "...\n");
                buffer[BUFFER_SIZE - 1] = '\0';
                ::printf("INFO: logwidget output truncated to %s", buffer);
            }

            if(g_ui.UI_ThreadID() != SYS::Thread::CallerID())
                _bufferMtx.Lock();

            *_bufferCur = (char)blockType;
            _bufferCur++;

            const size_t len = strlen(buffer);

            const size_t bufferLen = _bufferCur - &_buffer[0];
            if(_buffer.size() < bufferLen + len + 2) {
                // alternatively block the caller until next flush
                _buffer.resize(_buffer.size() + len + 2);
            }

            strncpy(_bufferCur, buffer, len);
            _bufferCur += len;

            *_bufferCur = '\0';
            _bufferCur++;

            *_bufferCur = BT_END;

            if(g_ui.UI_ThreadID() != SYS::Thread::CallerID())
                _bufferMtx.Unlock();
        }
    }

    void LogWidget::Flush() {
        static const QBrush bg_brushes[] = {
            QBrush(QColor(62, 62, 62)),
            QBrush(QColor(46, 46, 46)) // #2E2E2E
        };

        QBrush fg_brushes[] = {
            QBrush(QColor(0, 0, 0)),
            _defaultFgBrush
        };

        _bufferMtx.Lock();

        bool textChanged = false;

        char blockType, *bufferCur = &_buffer[0];
        while(BT_END != (blockType = *bufferCur)) {
            bufferCur++;

            QString string = QString(bufferCur);

            QTextCursor textCur(_ui.textBrowser->document());
            QTextBlockFormat bf = textCur.blockFormat();
            bf.setBackground(bg_brushes[_blockCnt % 2]);
            QTextCharFormat cf = textCur.charFormat();
            cf.setForeground(fg_brushes[blockType - 1]);
            textCur.movePosition(QTextCursor::End);
            textCur.setBlockFormat(bf);
            textCur.insertText(string, cf);
            _ui.textBrowser->textCursor().movePosition(QTextCursor::End);

            bufferCur += string.length() + 1;
            _blockCnt++;

            textChanged = true;
        }

        _buffer[0] = BT_END;
        _bufferCur = &_buffer[0];

        _bufferMtx.Unlock();

        if(textChanged) _ui.textBrowser->ensureCursorVisible();
    }

    void LogWidget::sys_printf(const char* format, ...) {
            va_list args;
            va_start(args, format);
            printf(BT_SYSTEM, format, args);
            va_end(args);
    }

    void LogWidget::printf(const char* format, ...) {
            va_list args;
            va_start(args, format);
            printf(BT_OPERATOR, format, args);
            va_end(args);
    }


} // namespace UI