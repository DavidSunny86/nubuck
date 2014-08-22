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

struct TF_Char {
    int     id;
    int     x;
    int     y;
    int     width;
    int     height;
    float   xoffset;
    float   yoffset;
    float   xadvance;
    int     page;
    int     chnl;
};

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
private:
    Info    _info;
    Common  _common;
    Page*   _pages;
    TF_Char _chars[256];

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
    const TF_Char&  GetChar(char c) const;

    const TF_Char* const GetChars() const;

    Texture*        GetPageTexture(int page) const;

    // called by TexFontManager. requires valid gl context
    void LoadTextures();
};

class SDTexFont {
public:
    struct Info {
        std::string face;
    };

    struct Common {
        float   lineHeight;
        int     scaleW;
        int     scaleH;
    };
private:
    Info    _info;
    Common  _common;
    TF_Char _chars[256];

    std::string _baseName; // filename minus extension

    GEN::Pointer<Texture> _texture;

    void ParseInfo(COM::ItTokenizer& toks);
    void ParseCommon(COM::ItTokenizer& toks);
    void ParseChars(COM::ItTokenizer& toks);
public:
    ~SDTexFont();

    // by convention filename has the form f.ttf_sdf.txt.
    // the function then expects the texture to have
    // filename f.ttf_sdf.tga
    void LoadFromFile(const std::string& filename);

    const Info&     GetInfo() const;
    const Common&   GetCommon() const;
    const TF_Char&  GetChar(char c) const;

    const TF_Char* const GetChars() const;

    Texture*        GetPageTexture();

    // called by TexFontManager. requires valid gl context
    void LoadTextures();
};

class TexFontManager : public GEN::Singleton<TexFontManager> {
    friend class GEN::Singleton<TexFontManager>;
private:
    std::map<std::string, GEN::Pointer<TexFont> >       _texFonts;
    std::map<std::string, GEN::Pointer<SDTexFont>   >   _sdtexFonts;
public:
    TexFont&    GetTexFont(const std::string& filename);
    SDTexFont&  GetSDTexFont(const std::string& filename);

    // called by renderer. requires valid gl context
    void LoadTextures();
};

} // namespace R