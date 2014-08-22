#include <stdio.h>
#include <map>
#include <Nubuck\generic\singleton.h>
#include <Nubuck\common\common.h>
#include <common\string_helper.h>
#include "texfont.h"

namespace R {

void ExpectField(COM::ItTokenizer& toks, const char* name, int& val) {
    toks.Expect(name);
    toks.Expect("=");
    toks.ExpectInt(val);
}

void ExpectField(COM::ItTokenizer& toks, const char* name, float& val) {
    toks.Expect(name);
    toks.Expect("=");
    toks.ExpectFloat(val);
}

void ExpectField(COM::ItTokenizer& toks, const char* name, std::string& val) {
    toks.Expect(name);
    toks.Expect("=");
    toks.ExpectStr(val);
}

void DestroyTexFont(TexFont& texFont) {
    delete[] texFont.pages;
}

/*
================================================================================
    AngelCode BMFont Tool
================================================================================
*/

void BMF_ParseInfo(COM::ItTokenizer& toks, TF_Info& info) {
    toks.Expect("info");

    ExpectField(toks, "face", info.face);
    ExpectField(toks, "size", info.size);
    ExpectField(toks, "bold", info.bold);
    ExpectField(toks, "italic", info.italic);
    ExpectField(toks, "charset", info.charset);
    ExpectField(toks, "unicode", info.unicode);
    ExpectField(toks, "stretchH", info.stretchH);
    ExpectField(toks, "smooth", info.smooth);
    ExpectField(toks, "aa", info.aa);

    toks.Expect("padding");
    toks.Expect("=");
    toks.ExpectInt(info.padding[0]);
    toks.Expect(",");
    toks.ExpectInt(info.padding[1]);
    toks.Expect(",");
    toks.ExpectInt(info.padding[2]);
    toks.Expect(",");
    toks.ExpectInt(info.padding[3]);

    toks.Expect("spacing");
    toks.Expect("=");
    toks.ExpectInt(info.spacing[0]);
    toks.Expect(",");
    toks.ExpectInt(info.spacing[1]);

    ExpectField(toks, "outline", info.outline);
}

void BMF_ParseCommon(COM::ItTokenizer& toks, TF_Common& common) {
    toks.Expect("common");

    ExpectField(toks, "lineHeight", common.lineHeight);
    ExpectField(toks, "base", common.base);
    ExpectField(toks, "scaleW", common.scaleW);
    ExpectField(toks, "scaleH", common.scaleH);
    ExpectField(toks, "pages", common.pages);
    ExpectField(toks, "packed", common.packed);
    ExpectField(toks, "alphaChnl", common.alphaChnl);
    ExpectField(toks, "redChnl", common.redChnl);
    ExpectField(toks, "greenChnl", common.greenChnl);
    ExpectField(toks, "blueChnl", common.blueChnl);
}

// precond: pages is array of size numPages
void BMF_ParsePages(COM::ItTokenizer& toks, int numPages, TF_Page* pages) {
    for(unsigned i = 0; i < numPages; ++i) {
        TF_Page& page = pages[i];

        toks.Expect("page");

        ExpectField(toks, "id", page.id);
        
        std::string filename;
        ExpectField(toks, "file", filename);
        page.file = common.BaseDir() + "Textures\\Fonts\\" + filename;
    }
}

// precond: chars is array of size 256
void BMF_ParseChars(COM::ItTokenizer& toks, TF_Char* chars) {
    toks.Expect("chars");

    int numChars = 0;
    ExpectField(toks, "count", numChars);

    for(unsigned i = 0; i < numChars; ++i) {
        toks.Expect("char");

        int id = 0;
        ExpectField(toks, "id", id);

        if(255 < id) {
            common.printf("ERROR - char id outside range\n");
            continue;
        }

        TF_Char& c = chars[id];
        c.id = id;

        ExpectField(toks, "x", c.x);
        ExpectField(toks, "y", c.y);
        ExpectField(toks, "width", c.width);
        ExpectField(toks, "height", c.height);
        ExpectField(toks, "xoffset", c.xoffset);
        ExpectField(toks, "yoffset", c.yoffset);
        ExpectField(toks, "xadvance", c.xadvance);
        ExpectField(toks, "page", c.page);
        ExpectField(toks, "chnl", c.chnl);
    }
}

void BMF_LoadFromFile(const std::string& filename, TexFont& texFont) {
    FILE* file = fopen(filename.c_str(), "r");
    if(!file) {
        common.printf("ERROR - BMF_LoadFromFile: ");
        common.printf("unable to open file '%s'", filename.c_str());
        Crash();
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char* buffer = new char[size];

    if(1 != fread(buffer, size, 1, file) && ferror(file)) {
        common.printf("ERROR - BMF_LoadFromFile: ");
        common.printf("unable to read file '%s'", filename.c_str());
        Crash();
    }

    COM::ItTokenizer toks(buffer, " \t\n");
    toks.SetName("BMF tokenizer");

    BMF_ParseInfo(toks, texFont.info);
    BMF_ParseCommon(toks, texFont.common);

    texFont.pages = new TF_Page[texFont.common.pages];

    BMF_ParsePages(toks, texFont.common.pages, texFont.pages);
    BMF_ParseChars(toks, texFont.chars);

    delete[] buffer;

    fclose(file);
}

/*
================================================================================
    Lonesock's Signed Distance Font Tool
================================================================================
*/

void SDF_ParseInfo(COM::ItTokenizer& toks, TF_Info& info) {
    toks.Expect("info");

    ExpectField(toks, "face", info.face);
}

void SDF_ParseCommon(COM::ItTokenizer& toks, TF_Common& common) {
    toks.Expect("common");

    ExpectField(toks, "lineHeight", common.lineHeight);
    ExpectField(toks, "scaleW", common.scaleW);
    ExpectField(toks, "scaleH", common.scaleH);
}

// precond: chars is array of size 256
void SDF_ParseChars(COM::ItTokenizer& toks, TF_Char* chars) {
    toks.Expect("chars");

    int numChars = 0;
    ExpectField(toks, "count", numChars);

    for(unsigned i = 0; i < numChars; ++i) {
        toks.Expect("char");

        int id = 0;
        ExpectField(toks, "id", id);

        if(255 < id) {
            common.printf("ERROR - char id outside range\n");
            continue;
        }

        TF_Char& c = chars[id];
        c.id = id;

        ExpectField(toks, "x", c.x);
        ExpectField(toks, "y", c.y);
        ExpectField(toks, "width", c.width);
        ExpectField(toks, "height", c.height);
        ExpectField(toks, "xoffset", c.xoffset);
        ExpectField(toks, "yoffset", c.yoffset);
        ExpectField(toks, "xadvance", c.xadvance);
        ExpectField(toks, "page", c.page);
        ExpectField(toks, "chnl", c.chnl);
    }
}

void SDF_LoadFromFile(const std::string& filename, TexFont& texFont) {
    std::string baseName = COM::StripFileExtension(filename);

    FILE* file = fopen(filename.c_str(), "r");
    if(!file) {
        common.printf("ERROR - SDF_LoadFromFile: ");
        common.printf("unable to open file '%s'", filename.c_str());
        Crash();
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char* buffer = new char[size];

    if(1 != fread(buffer, size, 1, file) && ferror(file)) {
        common.printf("ERROR - SDF_LoadFromFile: ");
        common.printf("unable to read file '%s'", filename.c_str());
        Crash();
    }

    COM::ItTokenizer toks(buffer, " \t\n");
    toks.SetName("SDF tokenizer");

    SDF_ParseInfo(toks, texFont.info);
    SDF_ParseCommon(toks, texFont.common);
    SDF_ParseChars(toks, texFont.chars);

    texFont.common.pages = 1;
    texFont.pages = new TF_Page[1];
    texFont.pages[0].id = 0;
    texFont.pages[0].file = baseName + ".tga";

    delete[] buffer;

    fclose(file);
}

/*
================================================================================
    TexFontManager
================================================================================
*/

class TexFontManager : public GEN::Singleton<TexFontManager> {
    friend class GEN::Singleton<TexFontManager>;
private:
    struct Item {
        Item*       next;
        std::string filename;
        TexFont     texFont;
    } *_items;

    void Clear() {
        Item *temp, *it = _items;
        while(it) {
            temp = it->next;
            DestroyTexFont(it->texFont);
            delete it;
            it = temp;
        }
    }
public:
    TexFontManager() : _items(NULL) { }
    ~TexFontManager() { Clear(); }

    TexFont& FindTexFont(const TF_Type::Enum type, const std::string& filename) {
        Item* it = NULL;
        
        it = _items;
        while(it) {
            if(filename == it->filename) return it->texFont;
            it = it->next;
        }

        it = new Item;
        it->filename = filename;

        if(TF_Type::BMFont == type) BMF_LoadFromFile(filename, it->texFont);
        else if(TF_Type::SDFont == type) SDF_LoadFromFile(filename, it->texFont);
        else {
            common.printf("TexManager::FindTexFont: unknown type");
            Crash();
        }

        it->next = _items;
        _items = it;

        return it->texFont;
    }
};

const TexFont& FindTexFont(const TF_Type::Enum type, const std::string& filename) {
    return TexFontManager::Instance().FindTexFont(type, filename);
}

} // namespace R
