#pragma once
#include <frontend/layout/shell.hpp>

class CRibbonShell : public IShell {
    public:
        CBaseView* render( CBaseView* active ) override;
        void       add_nav_item( CBaseView::ViewItem item ) override;

    private:
        std::vector<CBaseView::ViewItem> m_items;
};
