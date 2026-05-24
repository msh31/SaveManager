#pragma once

struct FontData {
        std::string_view name;
        float            size;
        bool             is_icon;
        bool             is_default;
};

class CFontManager {
    public:
        void load_from_memory( FontData fd, void* data, int data_len );
        // void load_from_disk(std::string font_name, float font_size, fs::path path);

        std::optional<ImFont*> get( std::string_view font_name );

    private:
        std::unordered_map<std::string, ImFont*> m_fonts;
        std::vector<FontData>                    m_font_data;
};
