#pragma once

#include <stdio.h>
#include <string>

#include <Nubuck\nubuck_api.h>

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

namespace COM {

class ItTokenizer {
private:
    const char* start;
    const char* end;

    const char* name;
public:
    struct Token {
        const char* start;
        const char* end;

        Token() { }
        Token(const char* start, const char* end);
    };

    const char* const string;
    const char* const delim;

    ItTokenizer(const char* string, const char* delim);

    void        SetName(const char* name); // used for error messages

    Token       NextToken();
    bool        IsValid(const Token& tok) const;

    unsigned    Length(const Token& tok) const;
    unsigned    StartIndex(const Token& tok) const;
    unsigned    EndIndex(const Token& tok) const;

    Token       Expect(const char* name);
    Token       ExpectInt(int& val);
    Token       ExpectFloat(float& val);
    Token       ExpectStr(std::string& val);
};

} // namespace COM

class Common {
private:
    FILE* _logfile;

    std::string _baseDir;
public:
    Common(void);
    ~Common(void);

    void Init(int argc, char* argv[]);

    float RandomFloat(float min, float max) const;

    // contains trailing delimiter, eg.
    // 'C:\Libraries\LEDA\'
    const std::string& BaseDir(void) const;

    const char* GetEnvVar(const std::string& name) const;

    void printf(const char* format, ...);
};

NUBUCK_API void COM_printf(const char* format, ...);

extern Common common;

void Crash(void);
