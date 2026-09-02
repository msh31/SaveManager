#pragma once

struct CMenuBarItem {
        const char* icon;
        const char* label;
        std::function<void( )> callback = nullptr;
        bool* state = nullptr;
        std::optional<ImVec4> active_color;
};

struct CMenuBarGroup {
        const char* label;
        std::vector<CMenuBarItem> items;
};

class CMenuBar {
    public:
        void add_group( CMenuBarGroup group );
        void render( );

    private:
        void render_item( const CMenuBarItem& item, float btn_h );

        std::vector<CMenuBarGroup> m_groups;
};
