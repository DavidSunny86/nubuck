#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include <string>

#include "common.h"

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

void Common::Init(int argc, char* argv[]) {
    _logfile = fopen("logfile.txt", "w");

    char delim = 0;
#ifdef _WIN32
    delim = '\\';
#endif
    assert(0 != delim);

    const char* s;
    if(s = getenv("LEDA_DIR")) {
        _baseDir = s;
        if(delim != _baseDir.back()) _baseDir += delim;
    }
    else {
        common.printf("INFO - environment variable 'LEDA_DIR' not set.\n");
        _baseDir = DirOf(argv[0]);
    }

#ifdef NSIGHT_BUILD
    _baseDir = "C:\\Users\\cj\\AppData\\Roaming\\NVIDIA Corporation\\Nsight\\Monitor\\Mirror\\cj-desktop\\c\\libraries\\leda\\leda-6.4\\";
#endif

    _baseDir += std::string("res") + delim;
    printf("base directory is '%s'.\n", _baseDir.c_str());
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

void Common::printf(const char* format, ...) {
    if(_logfile) {
        va_list args;
        va_start(args, format);
        vfprintf(_logfile, format, args);
        va_end(args);
        fflush(_logfile);
    }
}

void Crash(void) {
    exit(-1);
}
