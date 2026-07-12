#include "font_manager.hpp"

static const ImWchar icon_ranges[] = { 0xf000, 0xf2e0, 0 };

void CFontManager::load_from_memory( FontData fd, void* data, int data_len ) {
    ImGuiIO& io = ImGui::GetIO( );
    ImFontConfig cfg;                 //???
    cfg.FontDataOwnedByAtlas = false; // imgui will try to free mem of an embedded header which will fail

    ImFont* font = nullptr;
    if ( fd.is_icon ) {
        cfg.MergeMode = true;
        cfg.GlyphMinAdvanceX = fd.size;
        font = io.Fonts->AddFontFromMemoryTTF( data, data_len, fd.size, &cfg, icon_ranges );
    } else {
        font = io.Fonts->AddFontFromMemoryTTF( data, data_len, fd.size, &cfg );
    }
    if ( font == nullptr ) throw std::runtime_error( "Tried to load invalid font!" );
    if ( fd.is_default ) io.FontDefault = font;
    m_fonts.insert( { fd.name.data( ), font } );
    m_font_data.push_back( fd );
}

std::optional<ImFont*> CFontManager::get_font( std::string_view font_name ) {
    if ( m_fonts.empty( ) ) return std::nullopt;

    if ( auto it = m_fonts.find( std::string( font_name ) ); it != m_fonts.end( ) ) {
        return it->second;
    }
    return std::nullopt;
}

CFontManager& CFontManager::get( ) {
    static CFontManager instance;
    return instance;
}

CFontManager::CFontManager( ) {}
