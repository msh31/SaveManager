#include "editor_view.hpp"
#include <utils/utils.hpp>
#include <nfd.h>

#include <backend/font_manager/font_manager.hpp>
#include <frontend/notification/notification.hpp>
#include <frontend/childguard.hpp>

/*
    TODO LIST

    1. make text bigger (need to work on a size setter in the font manager)
*/

void CEditorView::render( ) {
    ImGui::Text( "Save Editor" ); //1.

    bool file_empty = file_path.empty( );

    if ( file_empty ) {
        ChildGuard toolbar( "", { 0.0f, 0.0f } );

        if ( ImGui::Button( "Open savefile" ) ) {
            NFD_Init( );

            nfdu8char_t* outPath;
            nfdu8filteritem_t filters[2] = { { "GTA SAN ANDREAS SAVE FILE", "b" } };
            nfdopendialogu8args_t args = { 0 };
            args.filterList = filters;
            args.filterCount = 1;
            nfdresult_t result = NFD_OpenDialogU8_With( &outPath, &args );
            if ( result == NFD_OKAY ) {
                std::string path( outPath );
                NFD_FreePathU8( outPath );
                if ( m_san_andreas.open( path ) ) {
                    file_path = path;
                    Notify::show_notification( "Save Editor", "Save loaded successfully!", 3000 );
                } else {
                    Notify::show_notification( "Save Editor", "Save failed to load!", 3000 );
                }
            } else if ( result == NFD_CANCEL ) {
            } else {
                Notify::show_notification( "Save Editor", "Save failed to load!", 3000 );
            }

            NFD_Quit( );
        }
    } else {
    
        if ( ImGui::Button( "Save" ) ) {
            m_san_andreas.save( file_path );
            Notify::show_notification( "Save Editor", "Savegame changed saved succesfully!", 3000 );
        }
        ImGui::SameLine( );
        if ( ImGui::Button( "Close" ) ) {
            m_san_andreas.close( );
            m_san_andreas = { };
            file_path = { };
        }

        {
            ChildGuard sa_info( "Save Information", { 0.0f, 0.0f } );
            ImGui::Text( "Save Information" );

            ImGui::TextDisabled( "%s", utils::utf8_to_path( file_path ).string( ).c_str( ) ); // cursed

            ImGui::Text( "Save Name: %s", m_san_andreas.save_name.c_str( ) );
            ImGui::Text( "Save Version: %s", m_san_andreas.save_version.c_str( ) );
        }

        ImGui::SameLine( );

        {
            ChildGuard sa_stats( "Statistics", { 0.0f, 0.0f } );
            ImGui::Text( "Statistics" );

            ImGui::SetNextItemWidth( 200.0f );
            if ( ImGui::DragInt(
                     std::format( "Money ({})", m_san_andreas.money_displayed ).c_str( ), &m_san_andreas.money, 1.0f,
                     INT32_MIN, INT32_MAX ) ) {
                m_san_andreas.money_displayed = m_san_andreas.money;
            }
            ImGui::SetNextItemWidth( 200.0f );
            ImGui::DragFloat( "Health", &m_san_andreas.health, 1.0f, 0.0, m_san_andreas.max_health );
            ImGui::SetNextItemWidth( 200.0f );
            ImGui::DragFloat( "Armor", &m_san_andreas.armor, 1.0f, 0.0, m_san_andreas.max_armor );
        }

        {
            ChildGuard sa_flags( "Flags", { 0.0f, 0.0f } );
            ImGui::Text( "Flags" );

            ImGui::Checkbox( "Lose stuff after wasted", &m_san_andreas.lose_stuff_after_wasted );
            ImGui::SameLine( );
            ImGui::Checkbox( "Lose stuff after busted", &m_san_andreas.lose_stuff_after_busted );
            ImGui::Separator( );
            ImGui::Checkbox( "Free Wasted Once", &m_san_andreas.free_wasted_once );
            ImGui::SameLine( );
            ImGui::Checkbox( "Free Busted Once", &m_san_andreas.free_busted_once );
            ImGui::Separator( );
            ImGui::Checkbox( "Infinite Run", &m_san_andreas.infinite_run );
            ImGui::SameLine( );
            ImGui::Checkbox( "Fast Reload", &m_san_andreas.fast_reload );
            ImGui::SameLine( );
            ImGui::Checkbox( "Fireproof", &m_san_andreas.fireproof );
        }

         ImGui::SameLine( );

        {
            ChildGuard sa_collect( "Collectables", { 0.0f, 0.0f } );
            ImGui::Text( "Collectables" );

            if ( ImGui::Button( "Complete Spray Tags" ) ) std::ranges::fill( m_san_andreas.tag_statuses, 255 );
            ImGui::SameLine( );
            if ( ImGui::Button( "Reset Spray Tags" ) ) std::ranges::fill( m_san_andreas.tag_statuses, 0 );
            if ( ImGui::CollapsingHeader( std::format( "Spray Tags ({})", m_san_andreas.tag_count ).c_str( ) ) ) {
                for ( int i = 0; i < m_san_andreas.tag_count; i++ ) {
                    bool tagged = m_san_andreas.tag_statuses[i] > 0;
                    if ( ImGui::Checkbox( std::format( "Tag {}", i ).c_str( ), &tagged ) ) {
                        if ( tagged ) m_san_andreas.tag_statuses[i] = 255;
                        else
                            m_san_andreas.tag_statuses[i] = 0;
                    }
                    if ( ( i + 1 ) % 5 != 0 ) ImGui::SameLine( );
                }
            }
            ImGui::Separator( );
            if ( ImGui::Button( "Complete Unique Stunt Jumps" ) ) {
                std::ranges::fill( m_san_andreas.usj_found, true );
                std::ranges::fill( m_san_andreas.usj_done, true );
            }
            ImGui::SameLine( );
            if ( ImGui::Button( "Reset Unique Stunt Jumps" ) ) {
                std::ranges::fill( m_san_andreas.usj_found, false );
                std::ranges::fill( m_san_andreas.usj_done, false );
            }
            if ( ImGui::CollapsingHeader(
                     std::format( "Unique Stunt Jumps ({})", m_san_andreas.usj_count ).c_str( ) ) ) {
                for ( int i = 0; i < m_san_andreas.usj_count; i++ ) {
                    bool completed = m_san_andreas.usj_done[i] > 0;
                    if ( ImGui::Checkbox( std::format( "Stunt Jump {}", i ).c_str( ), &completed ) ) {
                        m_san_andreas.usj_done[i] = completed;
                        m_san_andreas.usj_found[i] = completed;
                    }
                    if ( ( i + 1 ) % 5 != 0 ) ImGui::SameLine( );
                }
            }
        }
    }
}
