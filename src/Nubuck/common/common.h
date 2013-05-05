#pragma once

#include <stdio.h>

#include <string>

#include <Nubuck\nubuck.h>

class Common : public ICommon {
private:
    FILE* _logfile;

    std::string _baseDir;
public:
    Common(void);
    ~Common(void);

    void Init(int argc, char* argv[]);

    float RandomFloat(float min, float max) const;

    // contains trailing delimiter, eg.
    // C:\Libraries\LEDA\ 
    const std::string& BaseDir(void) const;

    void printf(const char* format, ...) override;
};

extern Common common;

void Crash(void);
