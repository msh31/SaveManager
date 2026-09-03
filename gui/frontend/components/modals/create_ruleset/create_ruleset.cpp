#include "create_ruleset.hpp"
#include <utils/utils.hpp>
#include <frontend/notification/notification.hpp>


void CCreateRulesetModal::open( const Game& game, const std::function<void( const Game& )>& on_create ) {
    m_new_ruleset_text.clear( );
    m_file_list_ignore_rulset.clear( );

    if ( game.save_paths.empty( ) ) {
        SPDLOG_ERROR( "[CreateRulesetModal] Game '{}' has no save paths, refusing to open", game.game_name );
        return;
    }

    m_pending_ignore_game = game;
    m_pending_ignore_save_root = game.save_paths.front( );

    m_ignore_file = m_pending_ignore_save_root / ".savemgr-ignore";
    std::error_code ec;
    if ( fs::exists( m_ignore_file, ec ) && !ec ) {
        std::ifstream in( m_ignore_file );
        if ( in.is_open( ) ) {
            m_new_ruleset_text.assign( std::istreambuf_iterator<char>( in ), std::istreambuf_iterator<char>( ) );
        }
    }

    for ( const auto& sp : game.save_paths ) {
        ec.clear( );
        if ( !fs::is_directory( sp, ec ) || ec ) continue;

        for ( auto it = fs::recursive_directory_iterator( sp, fs::directory_options::skip_permission_denied, ec );
              it != fs::recursive_directory_iterator( ); it.increment( ec ) ) {
            if ( ec ) break;

            // Keep paths in their native representation. Converting through path::string()
            // throws on Windows when a filename is not representable in the active code page.
            auto path = it->path( ).lexically_relative( sp );
            if ( path.empty( ) || path == fs::path( ".savemgr-ignore" ) ) continue;
            m_file_list_ignore_rulset.emplace_back( path );
        }
    }

    m_on_create = on_create;
    request_open( );
}

void CCreateRulesetModal::render_content( ) {
    auto height = std::clamp(
        static_cast<float>( m_file_list_ignore_rulset.size( ) ) * ImGui::GetFrameHeightWithSpacing( ), 200.0f,
        500.0f );
    ImGui::BeginChild( "##Filter entries", ImVec2( 500, height ) );
    ImGui::Text( "File list" );
    ImGui::Separator( );

    for ( const auto& entry : m_file_list_ignore_rulset ) {
        std::string text = utils::path_to_utf8( entry );
        ImGui::PushID( text.c_str( ) );
        ImGui::Text( "%s", text.c_str( ) );
        ImGui::Separator( );
        ImGui::PopID( );
    }

    ImGui::Dummy( ImVec2( 0.0f, 5.0f ) );

    ImGui::InputTextMultiline(
        "##filter input", &m_new_ruleset_text, ImVec2( -FLT_MIN, ImGui::GetTextLineHeight( ) * 16 ) );

    if ( ImGui::Button( "Save" ) ) {
        if ( utils::atomic_write( m_ignore_file, m_new_ruleset_text ) ) {
            if ( m_on_create ) m_on_create( m_pending_ignore_game );
            Notify::show_notification( "Ignore ruleset", "Created ruleset successfully!", 2000 );
            ImGui::CloseCurrentPopup( );
        } else {
            Notify::show_notification( "Ignore ruleset", "Failed to create ruleset!", 2000 );
        }
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Cancel" ) ) {
        ImGui::CloseCurrentPopup( );
    }

    ImGui::EndChild( );
}
