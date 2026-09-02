#include "font_manager.hpp"
#include <logger.hpp>

static const ImWchar icon_ranges[] = { 0xf000, 0xf2e0, 0 };

void CFontManager::load_one( const FontData& fd ) {
    ImGuiIO& io = ImGui::GetIO( );
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false; // data is embedded/static and outlives the atlas

    ImFont* font = nullptr;
    if ( fd.is_icon ) {
        cfg.MergeMode = true;
        cfg.GlyphMinAdvanceX = fd.size;
        font = io.Fonts->AddFontFromMemoryTTF( fd.data, fd.data_len, fd.size, &cfg, icon_ranges );
    } else {
        font = io.Fonts->AddFontFromMemoryTTF( fd.data, fd.data_len, fd.size, &cfg );
    }
    if ( font == nullptr ) throw std::runtime_error( "Tried to load invalid font!" );
    if ( fd.is_default ) io.FontDefault = font;
    m_fonts.insert( { std::string( fd.name ), font } );
}

void CFontManager::load_all( const std::vector<FontData>& fonts ) {
    for ( const auto& fd : fonts )
        load_one( fd );
}

std::optional<ImFont*> CFontManager::get_font( std::string_view font_name ) {
    if ( auto it = m_fonts.find( std::string( font_name ) ); it != m_fonts.end( ) ) {
        return it->second;
    }
    return std::nullopt;
}

CFontManager& CFontManager::get( ) {
    static CFontManager instance;
    return instance;
}
