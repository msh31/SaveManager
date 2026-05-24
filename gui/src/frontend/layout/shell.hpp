#pragma once
#include <frontend/views/base_view.hpp>

class IShell {
    public:
        virtual ~IShell( )                             = default;
        virtual CBaseView* render( CBaseView* active ) = 0;
        virtual void       add_nav_item( CBaseView::ViewItem item ) {}
        virtual void       set_settings( CBaseView::ViewItem item ) { add_nav_item( item ); }
};
