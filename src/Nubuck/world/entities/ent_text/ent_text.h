#pragma once

#include <UI\outliner\outliner.h>
#include <world\entity.h>
#include <renderer\text\r_text.h>

namespace W {

class ENT_Text : public Entity {
private:
    UI::Outliner::itemHandle_t _outlinerItem;

    R::Text _text;
    R::Text _sdtext;
public:
    ENT_Text();

    UI::OutlinerView* CreateOutlinerView() override;

    void GetRenderJobs(R::RenderList& renderList);
};

} // namespace W