#pragma once

#include <stdio.h>

#include <string>

#include <Nubuck\nubuck.h>

#define COM_assert(expr) \
    do { \
	    if(!(expr)) { \
            common.printf("assertion failed: %s in %s, %d\n", #expr, __FILE__, __LINE__); \
            Crash(); \
	    } \
    } \
    while(0)

namespace COM {

    struct FileNotFoundException : std::exception { };
	struct IOException : std::exception { };
	struct InvalidFormatException : std::exception { };

    typedef unsigned char byte_t;

} // namespace COM

#define MAX_TOKEN 512

typedef struct ctoken_s {
	struct ctoken_s* next;
	char string[MAX_TOKEN];
	float f;
	int i;
} ctoken_t;

int COM_Tokenize(ctoken_t** tokens, const char* string, char term);
void COM_FreeTokens(ctoken_t* tokens);

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

    const char* GetEnvVar(const std::string& name) const;

    void printf(const char* format, ...) override;
};

extern Common common;

void Crash(void);
