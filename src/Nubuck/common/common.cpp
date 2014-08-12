#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include <string>

#include <system\winerror.h>
#include <Nubuck\common\common.h>
#include <UI\logwidget\logwidget.h>

namespace COM {

inline bool IsDelim(const char* delim, char c) {
    const char* cur = delim;
    while('\0' !=  *cur) {
        if(*cur == c) return true;
        cur++;
    }
    return false;
}

ItTokenizer::Token::Token(const char* start, const char* end)
    : start(start)
    , end(end)
{ }

ItTokenizer::ItTokenizer(const char* string, const char* delim)
    : string(string)
    , delim(delim)
    , start(string)
    , end(string)
{ }

ItTokenizer::Token ItTokenizer::NextToken() {
    const char* cur = end;
    while('\0' != *cur && IsDelim(delim, *cur)) cur++;
    start = cur;
    while('\0' != *cur && !IsDelim(delim, *cur)) cur++;
    end = cur;
    return Token(start, end);
}

bool ItTokenizer::IsValid(const Token& tok) const { return tok.end - tok.start; }
unsigned ItTokenizer::Length(const Token& tok) const { return tok.end - tok.start; }
unsigned ItTokenizer::StartIndex(const Token& tok) const { return tok.start - string; }
unsigned ItTokenizer::EndIndex(const Token& tok) const { return tok.end - string; }

} // namespace COM

static std::string DirOf(const std::string& path) {
    const char* delim = "/\\";
    return path.substr(0, path.find_last_of(delim));
}

Common common;

Common::Common(void) : _baseDir(), _logfile(NULL) {
}

Common::~Common(void) {
    if(_logfile) fclose(_logfile);
}

std::string CurrentDirectory(void) {
    DWORD bufferLength = 0;
    bufferLength = GetCurrentDirectoryA(0, NULL);
    CHECK_WIN_ERROR;
    assert(bufferLength);
    std::string buffer(bufferLength - 1, '0');
    bufferLength = GetCurrentDirectoryA(bufferLength, &buffer[0]);
    CHECK_WIN_ERROR;
    assert(bufferLength);
    return buffer;
}

void Common::Init(int argc, char* argv[]) {
    unsigned i = 0;

    _logfile = fopen("logfile.txt", "w");

    char delim = 0;
#ifdef _WIN32
    delim = '\\';
#endif
    assert(0 != delim);

    _baseDir = CurrentDirectory();

    bool ignoreLedaDir = false;
    i = 0;
    while(i < argc && !ignoreLedaDir) {
        if(!strcmp("--ignoreledadir", argv[i]))
            ignoreLedaDir = true;
        i++;
    }

    const char* s;
    if(s = getenv("LEDA_DIR")) {
        if(!ignoreLedaDir) _baseDir = s;
        common.printf("INFO - environment variable 'LEDA_DIR' set to '%s'\n", s);
    }
    else common.printf("INFO - environment variable 'LEDA_DIR' not set.\n");

    i = 0;
    while(i < argc - 1) {
        if(!strcmp("--basedir", argv[i]))
            _baseDir = DirOf(argv[i + 1]);
        i++;
    }

    if(delim != _baseDir.back()) _baseDir += delim;
    _baseDir += std::string("res") + delim;
    printf("INFO - base directory is '%s'.\n", _baseDir.c_str());
}

float Common::RandomFloat(float min, float max) const {
    float r = min + (max - min) * ((float)rand() / RAND_MAX);
    if(r < min) r = min;
    if(r > max) r = max;
    return r;
}

const std::string& Common::BaseDir(void) const {
    return _baseDir;
}

const char* Common::GetEnvVar(const std::string& name) const {
    return getenv(name.c_str());
}

void Common::printf(const char* format, ...) {
    static char buffer[2048];

    memset(buffer, 0, sizeof(buffer));
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    if(_logfile) {
        fprintf(_logfile, buffer);
        fflush(_logfile);
    }

    UI::LogWidget::Instance()->sys_printf(buffer);
}

void COM_printf(const char* format, ...) {
    // ...
}

void Crash(void) {
    exit(-1);
}
