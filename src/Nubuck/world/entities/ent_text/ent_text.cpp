#include <assert.h>
#include <renderer\texfont\texfont.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\quad\quad.h>
#include <world\entities\ent_text\ent_text.h>
#include <world\entities\ent_text\ent_text_outln.h>
#include <UI\userinterface.h>

COM::Config::Variable<int> cvar_r_textType("textType", 0);

namespace W {

static const char CHAR = 't';

ENT_Text::ENT_Text()
    : _outlinerItem(NULL)
{
    SetType(EntityType::ENT_TEXT);

    _outlinerItem = g_ui.GetOutliner().AddItem("", this);
    g_ui.GetOutliner().SetItemName(_outlinerItem, "Text");


    const std::string filename = common.BaseDir() + "Textures\\Fonts\\consolas_32.fnt";
    R::TexFont& texFont = R::TexFontManager::Instance().GetTexFont(filename);

    const std::string sdfilename = common.BaseDir() + "Textures\\Fonts\\consola.ttf_sdf.txt";
    R::SDTexFont& sdtexFont = R::TexFontManager::Instance().GetSDTexFont(sdfilename);

    _text.Rebuild(texFont, "Hello World\nHow are you?");
    _sdtext.Rebuild(sdtexFont, "Hello World\nHow are you?");
}

UI::OutlinerView* ENT_Text::CreateOutlinerView() {
    return new ENT_TextOutln();
}

void ENT_Text::GetRenderJobs(R::RenderList& renderList) {
    const std::string filename = common.BaseDir() + "Textures\\Fonts\\consolas_32.fnt";
    R::TexFont& texFont = R::TexFontManager::Instance().GetTexFont(filename);

    const std::string sdfilename = common.BaseDir() + "Textures\\Fonts\\consola.ttf_sdf.txt";
    R::SDTexFont& sdtexFont = R::TexFontManager::Instance().GetSDTexFont(sdfilename);

    if(0 == cvar_r_textType) _text.GetRenderJobs(texFont, renderList);
    else _sdtext.GetRenderJobs(sdtexFont, renderList);
}

} // namespace W