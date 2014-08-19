#include <assert.h>
#include <renderer\texfont\texfont.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\quad\quad.h>
#include <world\entities\ent_text\ent_text.h>
#include <world\entities\ent_text\ent_text_outln.h>
#include <UI\userinterface.h>

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

    _text.Rebuild(texFont, "Hello World\nHow are you?");
}

UI::OutlinerView* ENT_Text::CreateOutlinerView() {
    return new ENT_TextOutln();
}

void ENT_Text::GetRenderJobs(R::RenderList& renderList) {
    const std::string filename = common.BaseDir() + "Textures\\Fonts\\consolas_32.fnt";
    R::TexFont& texFont = R::TexFontManager::Instance().GetTexFont(filename);

    _text.GetRenderJobs(texFont, renderList);
}

} // namespace W