#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include <string>

#include <system\winerror.h>
#include <Nubuck\common\common.h>

int COM_Tokenize(ctoken_t** tokens, const char* string, char term) {
	ctoken_t* token;
	ctoken_t* head = NULL;
	ctoken_t* tail = NULL;
	bool reading = false;
	const char* strPtr = string;
	char* tokenPtr;
	int num = 0;
	int i;

	COM_assert(NULL != tokens);

	while(term != *strPtr) {
		if(' ' == *strPtr || '\t' == *strPtr) {
			if(reading) reading = false;
			strPtr++;
			continue;
		}
		if(!reading) {
			token = (ctoken_t*)malloc(sizeof(ctoken_t));
			if(!token) {
                common.printf("out of memory tokenizing string.\n");
				return -1;
			}
			// for(i = 0; i < MAX_TOKEN; ++i) token->string[i] = '\0';
			memset(token->string, 0, MAX_TOKEN);
			token->next = NULL;
			if(tail) tail->next = token;
			else head = token;
			tail = token;
			
			tokenPtr = token->string;
			reading = true;
		}
		*tokenPtr++ = *strPtr++;
	}

	token = head;
	while(NULL != token) {
		num++;
		sscanf(token->string, "%11f", &token->f);
		sscanf(token->string, "%d", &token->i);
		token = token->next;
	}

	*tokens = head;
	return num;
}

void COM_FreeTokens(ctoken_t* tokens) {
	ctoken_t* tmp;
	while(NULL != tokens) {
		tmp = tokens->next;
		free(tokens);
		tokens = tmp;
	}
}

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
