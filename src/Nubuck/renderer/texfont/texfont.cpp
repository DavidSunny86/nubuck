#include <stdio.h>
#include <Nubuck\common\common.h>
#include <renderer\textures\texture.h>
#include "texfont.h"

namespace R {

void ExpectField(COM::ItTokenizer& toks, const char* name, int& val) {
    toks.Expect(name);
    toks.Expect("=");
    toks.ExpectInt(val);
}

void ExpectField(COM::ItTokenizer& toks, const char* name, std::string& val) {
    toks.Expect(name);
    toks.Expect("=");
    toks.ExpectStr(val);
}

void TexFont::ParseInfo(COM::ItTokenizer& toks) {
    toks.Expect("info");

    ExpectField(toks, "face", _info.face);
    ExpectField(toks, "size", _info.size);
    ExpectField(toks, "bold", _info.bold);
    ExpectField(toks, "italic", _info.italic);
    ExpectField(toks, "charset", _info.charset);
    ExpectField(toks, "unicode", _info.unicode);
    ExpectField(toks, "stretchH", _info.stretchH);
    ExpectField(toks, "smooth", _info.smooth);
    ExpectField(toks, "aa", _info.aa);

    toks.Expect("padding");
    toks.Expect("=");
    toks.ExpectInt(_info.padding[0]);
    toks.Expect(",");
    toks.ExpectInt(_info.padding[1]);
    toks.Expect(",");
    toks.ExpectInt(_info.padding[2]);
    toks.Expect(",");
    toks.ExpectInt(_info.padding[3]);

    toks.Expect("spacing");
    toks.Expect("=");
    toks.ExpectInt(_info.spacing[0]);
    toks.Expect(",");
    toks.ExpectInt(_info.spacing[1]);

    ExpectField(toks, "outline", _info.outline);
}

void TexFont::ParseCommon(COM::ItTokenizer& toks) {
    toks.Expect("common");

    ExpectField(toks, "lineHeight", _common.lineHeight);
    ExpectField(toks, "base", _common.base);
    ExpectField(toks, "scaleW", _common.scaleW);
    ExpectField(toks, "scaleH", _common.scaleH);
    ExpectField(toks, "pages", _common.pages);
    ExpectField(toks, "packed", _common.packed);
    ExpectField(toks, "alphaChnl", _common.alphaChnl);
    ExpectField(toks, "redChnl", _common.redChnl);
    ExpectField(toks, "greenChnl", _common.greenChnl);
    ExpectField(toks, "blueChnl", _common.blueChnl);
}

void TexFont::ParsePages(COM::ItTokenizer& toks) {
    _pages = new Page[_common.pages];

    for(unsigned i = 0; i < _common.pages; ++i) {
        Page& page = _pages[i];

        toks.Expect("page");

        ExpectField(toks, "id", page.id);
        ExpectField(toks, "file", page.file);
    }
}

void TexFont::ParseChars(COM::ItTokenizer& toks) {
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

        Char& c = _chars[id];
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

TexFont::~TexFont() {
    delete[] _pages;
}

void TexFont::LoadFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if(!file) {
        common.printf("ERROR - TexFont::LoadFromFile: ");
        common.printf("unable to open file '%s'", filename);
        Crash();
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char* buffer = new char[size];

    if(1 != fread(buffer, size, 1, file) && ferror(file)) {
        common.printf("ERROR - TexFont::LoadFromFile: ");
        common.printf("unable to read file '%s'", filename);
        Crash();
    }

    COM::ItTokenizer toks(buffer, " \t\n");
    toks.SetName("TexFont tokenizer");

    ParseInfo(toks);
    ParseCommon(toks);
    ParsePages(toks);
    ParseChars(toks);

    delete[] buffer;

    fclose(file);
}

const TexFont::Info& TexFont::GetInfo() const { return _info; }

const TexFont::Common& TexFont::GetCommon() const { return _common; }

const TexFont::Page& TexFont::GetPage(int page) const { return _pages[page]; }

const TexFont::Char& TexFont::GetChar(char c) const { return _chars[c]; }

Texture* TexFont::GetPageTexture(int page) const {
    return _pages[page].texture.Raw();
}

void TexFont::LoadTextures() {
    for(int i = 0; i < _common.pages; ++i) {
        Page& page = _pages[i];
        if(!page.texture.IsValid()) {
            const std::string filename = common.BaseDir() + "Textures\\Fonts\\consolas_32_0.tga";
            page.texture = R::TextureManager::Instance().Get(filename);
        }
    }
}

TexFont& TexFontManager::GetTexFont(const std::string& filename) {
    std::map<std::string, GEN::Pointer<TexFont> >::iterator it = _texFonts.find(filename);
    if(_texFonts.end() == it) {
        GEN::Pointer<TexFont> texFont = GEN::MakePtr(new TexFont());
        texFont->LoadFromFile(filename.c_str());
        _texFonts[filename] = texFont;
        return *texFont;
    }
    return *it->second;
}

void TexFontManager::LoadTextures() {
    std::map<std::string, GEN::Pointer<TexFont> >::iterator it;
    for(it = _texFonts.begin(); _texFonts.end() != it; ++it) {
        it->second->LoadTextures();
    }
}

} // namespace R
