#pragma once

class CStatusBar {
    public:
        struct Item {
                const char*           label;
                const char*           value;
                std::optional<ImVec4> dot_color;
        };

        static float height( ) { return ImGui::GetTextLineHeight( ) + ImGui::GetStyle( ).WindowPadding.y * 2.f; }

        void add_left( Item item );
        void add_right( Item item );
        void render( );

    private:
        void render_item( const Item& item );

        std::vector<Item> m_left;
        std::vector<Item> m_right;
};
