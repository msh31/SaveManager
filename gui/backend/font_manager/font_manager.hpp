#pragma once

struct FontData {
        std::string_view name;
        void* data;
        int data_len;
        float size; // base/design pixel size at 100% display scale
        bool is_icon;
        bool is_default;
};

class CFontManager {
    public:
        void load_all( const std::vector<FontData>& fonts );

        std::optional<ImFont*> get_font( std::string_view font_name );
        static CFontManager& get( );

        // delete copy & move constructors since there must only be 1 instance
        CFontManager( const CFontManager& ) = delete;
        CFontManager& operator=( const CFontManager& ) = delete;
        CFontManager( CFontManager&& ) = delete;
        CFontManager& operator=( CFontManager&& ) = delete;

    private:
        CFontManager( ) = default;
        void load_one( const FontData& fd );

        std::unordered_map<std::string, ImFont*> m_fonts;
};
