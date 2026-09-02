#include "settings_view.hpp"
#include <utils/image_extensions.hpp>
#include <utils/utils.hpp>

#include <frontend/childguard.hpp>
#include <frontend/notification/notification.hpp>
#include <frontend/theme/theme.hpp>

CSettingsView::CSettingsView( CConfig& cfg ) : m_config( cfg ) {};

void CSettingsView::on_enter( ) {
    m_backgrounds.clear( );
    
    //unlikely to happen but guard it anyway
    if ( fs::exists( paths::backgrounds_dir( ) ) ) {
        for ( const auto& f :
              fs::directory_iterator( paths::backgrounds_dir( ), fs::directory_options::skip_permission_denied ) ) {
            if ( f.is_regular_file( ) && utils::is_image_file( f.path( ) ) ) {
                m_backgrounds.emplace_back( f.path( ).filename( ).string( ) );
            }
        }
    
        m_current_background = 0;
        auto it = std::find( m_backgrounds.begin( ), m_backgrounds.end( ), m_config.settings.bg_name );
        if ( it != m_backgrounds.end( ) ) {
            m_current_background = (int)std::distance( m_backgrounds.begin( ), it );
        }
    }
}

void CSettingsView::render( ) {
    static bool test = false;

    {
        ChildGuard appearance( "Appearance", { 300.0f, 0.0f } );
        ImGui::Text( "Appearance" );

        if ( ImGui::Checkbox( "Dark Mode", &m_config.settings.dark_mode ) ) {
            ThemeManager::apply_colors( m_config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light );
        }

        ImGui::Separator( );

        ImGui::Checkbox( "Use Shader", &test );

        const char* items[] = { "Apple",  "Banana",    "Cherry",     "Kiwi",      "Mango",
                                "Orange", "Pineapple", "Strawberry", "Watermelon" };
        static int item_current = 1;
        ImGui::ListBox( "##shader_list", &item_current, items, IM_COUNTOF( items ), 4 );

        ImGui::Separator( );

        if ( (int)m_backgrounds.size( ) <= 0 ) {
            auto str = std::format( "No backgrounds found, add some here: {}", paths::backgrounds_dir( ).string( ) );
            ImGui::TextWrapped( "%s", str.c_str( ) );
        } else {
            ImGui::Checkbox( "Custom Background", &m_config.settings.use_bg );

            if ( ImGui::BeginListBox( "##bg_list" ) ) {
                for ( int i = 0; i < (int)m_backgrounds.size( ); i++ ) {
                    bool is_selected = ( m_current_background == i );
                    if ( ImGui::Selectable( m_backgrounds[i].c_str( ), is_selected ) ) {
                        m_current_background = i;
                        m_config.settings.bg_name = m_backgrounds[i];
                    }
                }
                ImGui::EndListBox( );
            }
        }

        ImGui::Separator( );
    }

    ImGui::SameLine( );

    {
        ChildGuard config( "Configuration", { 150.0f, 0.0f } );
        ImGui::Text( "Configuration" );

        if ( ImGui::Button( "Open" ) ) {
            utils::open_in_file_manager( paths::config_dir( ).string( ).c_str( ) );
        }
        ImGui::SetItemTooltip( "Opens your file manager to the config directory." );

        ImGui::SameLine( );
        if ( ImGui::Button( "Save" ) ) {
            m_config.save( );
            Notify::show_notification( "Config", "Saved config!", 1500 );
        }
    }
}

void CSettingsView::on_exit( ) { m_backgrounds.clear( ); }

CSettingsView::~CSettingsView( ) {}
