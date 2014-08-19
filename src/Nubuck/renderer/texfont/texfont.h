#pragma once

#include <string>
#include <map>
#include <Nubuck\generic\pointer.h>
#include <Nubuck\generic\singleton.h>
#include <Nubuck\math\vector2.h>

namespace COM {

class ItTokenizer;

} // namespace COM

namespace R {

class Texture;

class TexFont {
public:
    struct Info {
        std::string face;
        int         size;
        int         bold;
        int         italic;
        std::string charset;
        int         unicode;
        int         stretchH;
        int         smooth;
        int         aa;
        int         padding[4];
        int         spacing[2];
        int         outline;
    };

    struct Common {
        int lineHeight;
        int base;
        int scaleW;
        int scaleH;
        int pages;
        int packed;
        int alphaChnl;
        int redChnl;
        int greenChnl;
        int blueChnl;
    };

    struct Page {
        int         id;
        std::string file;

        GEN::Pointer<Texture> texture;
    };

    struct Char {
        int id;
        int x;
        int y;
        int width;
        int height;
        int xoffset;
        int yoffset;
        int xadvance;
        int page;
        int chnl;
    };
private:
    Info    _info;
    Common  _common;
    Page*   _pages;
    Char    _chars[256];

    void ParseInfo(COM::ItTokenizer& toks);
    void ParseCommon(COM::ItTokenizer& toks);
    void ParsePages(COM::ItTokenizer& toks);
    void ParseChars(COM::ItTokenizer& toks);
public:
    ~TexFont();

    void LoadFromFile(const char* filename);

    const Info&     GetInfo() const;
    const Common&   GetCommon() const;
    const Page&     GetPage(int page) const;
    const Char&     GetChar(char c) const;

    Texture*        GetPageTexture(int page) const;

    // called by TexFontManager. requires valid gl context
    void LoadTextures();
};

class TexFontManager : public GEN::Singleton<TexFontManager> {
    friend class GEN::Singleton<TexFontManager>;
private:
    std::map<std::string, GEN::Pointer<TexFont> > _texFonts;
public:
    TexFont& GetTexFont(const std::string& filename);

    // called by renderer. requires valid gl context
    void LoadTextures();
};

} // namespace R