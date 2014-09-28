#include <assert.h>
#include <renderer\texfont\texfont.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\quad\quad.h>
#include <world\entities\ent_text\ent_text.h>
#include <world\entities\ent_text\ent_text_outln.h>
#include <UI\userinterface.h>

namespace W {

void ENT_Text::Rebuild() {
    const std::string filename = common.BaseDir() + "Textures\\Fonts\\consola.ttf_sdf.txt";
    const R::TexFont& texFont = R::FindTexFont(R::TF_Type::SDFont, filename);

    _text.Rebuild(texFont, _content, _refChar, _refCharSize);

    // recompute bbox
    const M::Vector2 contentSize = GetContentSize();
    const M::Vector3 size = M::Vector3(contentSize.x, contentSize.y, 0.1f);
    const M::Vector3 center = 0.5f * M::Vector3(contentSize.x, -contentSize.y, 0.0f);
    M::Box bbox = M::Box::FromCenterSize(center, size);
    SetBoundingBox(bbox);
}

ENT_Text::ENT_Text()
    : _outlinerItem(NULL)
    , _content("Hello, World!\nHow are you?")
    , _refChar('A')
    , _refCharSize(2.5f)
{
    SetType(EntityType::ENT_TEXT);

    _outlinerItem = g_ui.GetOutliner().AddItem("", this);
    g_ui.GetOutliner().SetItemName(_outlinerItem, "Text");

    Rebuild();
}

UI::OutlinerView* ENT_Text::CreateOutlinerView() {
    return new ENT_TextOutln();
}

const M::Vector2& ENT_Text::GetContentSize() const {
    return _text.GetSize();
}

void ENT_Text::SetContent(const std::string& content) {
    _content = content;
    Rebuild();
}

void ENT_Text::SetContentScale(const char refChar, const float refCharSize) {
    _refChar = refChar;
    _refCharSize = refCharSize;
    Rebuild();
}

void ENT_Text::GetRenderJobs(R::RenderList& renderList) {
    _text.GetRenderJobs(GetObjectToWorldMatrix(), renderList);
}

} // namespace W