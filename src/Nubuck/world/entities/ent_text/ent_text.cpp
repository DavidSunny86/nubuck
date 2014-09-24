#include <assert.h>
#include <renderer\texfont\texfont.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\quad\quad.h>
#include <world\entities\ent_text\ent_text.h>
#include <world\entities\ent_text\ent_text_outln.h>
#include <UI\userinterface.h>

namespace W {

ENT_Text::ENT_Text()
    : _outlinerItem(NULL)
{
    SetType(EntityType::ENT_TEXT);

    _outlinerItem = g_ui.GetOutliner().AddItem("", this);
    g_ui.GetOutliner().SetItemName(_outlinerItem, "Text");

    const std::string filename = common.BaseDir() + "Textures\\Fonts\\consola.ttf_sdf.txt";
    const R::TexFont& texFont = R::FindTexFont(R::TF_Type::SDFont, filename);

    _text.Rebuild(texFont, "Hello World\nHow are you?");
}

UI::OutlinerView* ENT_Text::CreateOutlinerView() {
    return new ENT_TextOutln();
}

const M::Vector2& ENT_Text::GetContentSize() const {
    return _text.GetSize();
}

void ENT_Text::SetContent(const std::string& content) {
    const std::string filename = common.BaseDir() + "Textures\\Fonts\\consola.ttf_sdf.txt";
    const R::TexFont& texFont = R::FindTexFont(R::TF_Type::SDFont, filename);

    _text.Rebuild(texFont, content);
}

void ENT_Text::GetRenderJobs(R::RenderList& renderList) {
    _text.GetRenderJobs(GetObjectToWorldMatrix(), renderList);
}

} // namespace W