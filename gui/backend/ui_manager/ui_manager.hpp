#pragma once
#include <frontend/layout/shell.hpp>
#include <frontend/views/base_view.hpp>

#include <frontend/components/menubar/menubar.hpp>
#include <frontend/components/statusbar/statusbar.hpp>

class CUIManager {
    public:
        struct ViewConfig {
                std::unique_ptr<CBaseView> view;
                const char* icon;
                const char* label;
        };

        explicit CUIManager( std::unique_ptr<IShell> shell );

        CBaseView* add_view( ViewConfig cfg );
        void set_settings_view( ViewConfig cfg );
        void set_menubar( CMenuBar menubar );
        void set_statusbar( CStatusBar statusbar );
        void render( );

        CBaseView* get_active_view( ) const { return m_active_view; }
        void set_active_view( CBaseView* view );

    private:
        std::unique_ptr<IShell> m_shell;
        CBaseView* m_active_view = nullptr;
        std::vector<std::unique_ptr<CBaseView>> m_views;

        std::optional<CMenuBar> m_menubar;
        std::optional<CStatusBar> m_statusbar;
};
