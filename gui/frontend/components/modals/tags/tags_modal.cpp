#include "tags_modal.hpp"
#include <utils/utils.hpp>
#include <frontend/notification/notification.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void CTagsModal::open( const Game& game, const fs::path& backup, const std::vector<std::string>& tag_list,
    const std::function<void( const std::string&, const std::vector<std::string>& )>& on_saved ) {
    m_new_tag_input.clear( );
    m_pending_rename_game = game;
    m_pending_rename_backup = backup;
    m_pending_tags = tag_list;
    m_on_saved = on_saved;
    request_open( );
}

void CTagsModal::render_content() {
    ImGui::Text( "%s", utils::path_to_utf8( m_pending_rename_backup.filename( ) ).c_str( ) );
    ImGui::Separator( );

    int remove_index = -1;
    for ( size_t i = 0; i < m_pending_tags.size( ); i++ ) {
        ImGui::PushID( static_cast<int>( i ) );
        ImGui::Text( "%s", m_pending_tags[i].c_str( ) );
        ImGui::SameLine( );
        if ( ImGui::SmallButton( "x" ) ) remove_index = static_cast<int>( i );
        ImGui::PopID( );
    }
    if ( remove_index >= 0 ) m_pending_tags.erase( m_pending_tags.begin( ) + remove_index );

    if ( m_pending_tags.empty( ) ) ImGui::TextDisabled( "No tags yet" );

    ImGui::Separator( );

    bool add_tag = ImGui::InputText( "##new_tag", &m_new_tag_input, ImGuiInputTextFlags_EnterReturnsTrue );
    //ImGui::SameLine( );
    add_tag = ImGui::Button( "Add" ) || add_tag;
    if ( add_tag && !m_new_tag_input.empty( ) ) {
        if ( std::ranges::find( m_pending_tags, m_new_tag_input ) == m_pending_tags.end( ) )
            m_pending_tags.push_back( m_new_tag_input );
        m_new_tag_input.clear( );
    }

    ImGui::Dummy( ImVec2( 0, 5.0f ) );
    if ( ImGui::Button( "Save" ) ) {
        std::string backup_filename_utf8 = utils::path_to_utf8( m_pending_rename_backup.filename( ) );
        auto result = save_tags( m_pending_rename_game.game_name, backup_filename_utf8, m_pending_tags );
        if ( result.has_value( ) && *result ) {
            m_on_saved( backup_filename_utf8, m_pending_tags );
        } else {
            Notify::show_notification( "Tags", "Failed to save tags!", 1500 );
        }
        ImGui::CloseCurrentPopup( );
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Cancel" ) ) {
        ImGui::CloseCurrentPopup( );
    }
}

std::unordered_map<std::string, std::vector<std::string>> CTagsModal::load_tags( const std::string& game ) {
    std::unordered_map<std::string, std::vector<std::string>> tags;
    std::string file_name = ( paths::backup_dir( ) / utils::sanitize_filename_path( game ) / "tags.json" ).string( );
    if ( !fs::exists( file_name ) ) return { };

    std::ifstream in( file_name );
    if ( !in.is_open( ) ) {
        SPDLOG_ERROR( "Failed to load tags for {}!", game );
        return { };
    }

    json data;
    try {
        data = json::parse( in );
        for ( const auto& entry : data.items( ) ) {
            tags[entry.key( )] = entry.value( ).get<std::vector<std::string>>( );
        }
    } catch ( json::exception& ex ) {
        SPDLOG_ERROR( "tag parsing error: {}", ex.what( ) );
        return { };
    }

    return tags;
}

std::expected<bool, SMError>
CTagsModal::save_tags( const std::string& game, const std::string& filename, const std::vector<std::string>& tags ) {
    std::string file_name = ( paths::backup_dir( ) / utils::sanitize_filename_path( game ) / "tags.json" ).string( );

    json data = load_tags( game );
    data[filename] = tags;

    if ( data[filename].empty( ) ) {
        data.erase( filename );
    }

    if ( data.empty( ) ) {
        fs::remove( file_name );
        return true;
    }

    return utils::atomic_write( file_name, data.dump( 4 ) );
}