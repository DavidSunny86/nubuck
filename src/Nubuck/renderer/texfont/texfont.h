#pragma once

#include <string>

namespace R {

struct TF_Type {
    enum Enum {
        BMFont = 0,
        SDFont
    };
};

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

struct TF_Info {
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

struct TF_Common {
    float   lineHeight;
    int     base;
    int     scaleW;
    int     scaleH;
    int     pages;
    int     packed;
    int     alphaChnl;
    int     redChnl;
    int     greenChnl;
    int     blueChnl;
};

struct TF_Page {
    int         id;
    std::string file;
};

struct TexFont {
    TF_Type::Enum   type;
    TF_Info         info;
    TF_Common       common;
    TF_Page*        pages;
    TF_Char         chars[256];
};

const TexFont& FindTexFont(const TF_Type::Enum type, const std::string& filename);

} // namespace R