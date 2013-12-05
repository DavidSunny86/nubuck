#pragma once

#include <stdio.h>
#include <generic\uncopyable.h>

namespace COM {

    class FileHandle : GEN::Uncopyable {
    private:
        FILE* _handle;
    public:
        explicit FileHandle(FILE* handle) : _handle(handle) { }
        ~FileHandle() { if(_handle) fclose(_handle); }

        FILE* Handle() { return _handle; }

        FILE* Release() {
            FILE* handle = _handle;
            _handle = NULL;
            return handle;
        }
    };

} // namespace COM