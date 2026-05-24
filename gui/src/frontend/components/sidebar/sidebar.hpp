#pragma once
#include <frontend/views/base_view.hpp>

class CSideBar {
    public:
        CBaseView* render( CBaseView* active );
        void       add_item( CBaseView::ViewItem item );
        void       set_settings_view( CBaseView* view ); // a little odd but i cant think of anything better right now

    private:
        bool nav_button( const char* icon, const char* label, bool active, float width );

        std::vector<CBaseView::ViewItem> m_items;
        CBaseView*                       m_settings;
};
