#include "create_ruleset.hpp"
#include <utils/utils.hpp>
#include <frontend/notification/notification.hpp>


void CCreateRulesetModal::open( const Game& game, const std::function<void( const Game& )>& on_create ) {
    m_new_ruleset_text.clear( );
    m_file_list_ignore_rulset.clear( );
    m_pending_ignore_game = game;
    m_pending_ignore_save_root = game.save_paths.front( );

    m_ignore_file = m_pending_ignore_save_root / ".savemgr-ignore";
    if ( fs::exists( m_ignore_file ) ) {
        std::ifstream in( m_ignore_file );
        m_new_ruleset_text.assign( std::istreambuf_iterator<char>( in ), std::istreambuf_iterator<char>( ) );
        in.close( );
    }

    for ( const auto& sp : game.save_paths ) {
        for ( const auto& f : fs::recursive_directory_iterator( sp, fs::directory_options::skip_permission_denied ) ) {
            auto path = fs::relative( f.path( ), sp ).string( );
            if ( path == ".savemgr-ignore" ) continue;
            m_file_list_ignore_rulset.emplace_back( path );
        }
    }

    m_on_create = on_create;
    request_open( );
}

void CCreateRulesetModal::render_content( ) {
    auto height = std::clamp( m_file_list_ignore_rulset.size( ) * ImGui::GetFrameHeightWithSpacing( ), 200.0f, 500.0f );
    ImGui::BeginChild( "##Filter entries", ImVec2( 500, height ) );
    ImGui::Text( "File list" );
    ImGui::Separator( );

    for ( const auto& entry : m_file_list_ignore_rulset ) {
        ImGui::PushID( entry.c_str( ) );
        std::string text = std::format( "{}", utils::path_to_utf8( utils::utf8_to_path( entry.string( ) ) ) );
        ImGui::Text( "%s", text.c_str( ) );
        ImGui::Separator( );
        ImGui::PopID( );
    }

    ImGui::Dummy( ImVec2( 0.0f, 5.0f ) );

    ImGui::InputTextMultiline("##filter input", &m_new_ruleset_text, ImVec2( -FLT_MIN, ImGui::GetTextLineHeight( ) * 16 ) );

    if ( ImGui::Button( "Save" ) ) {
        utils::atomic_write( m_ignore_file, m_new_ruleset_text );
        m_on_create( m_pending_ignore_game );
        ImGui::CloseCurrentPopup( );
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Cancel" ) ) {
        ImGui::CloseCurrentPopup( );
    }

    ImGui::EndChild( );
}