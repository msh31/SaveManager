#pragma once
#include <frontend/components/sidebar/sidebar.hpp>
#include <frontend/layout/shell.hpp>

class CSidebarShell : public IShell {
    public:
        CBaseView* render( CBaseView* active ) override;
        void       add_nav_item( CBaseView::ViewItem item ) override;
        void       set_settings( CBaseView::ViewItem item ) override;

    private:
        CSideBar m_sidebar;
};
