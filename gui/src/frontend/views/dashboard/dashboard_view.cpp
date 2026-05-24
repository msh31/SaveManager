#include "dashboard_view.hpp"
#include <backend/utils.hpp>
#include <frontend/notification/notification.hpp>

void CDashboardView::on_enter( ) {};

void CDashboardView::render( ) {
    ImGuiTreeNodeFlags tree_flag = ImGuiTreeNodeFlags_DefaultOpen;
    if ( ImGui::TreeNodeEx( "Inputs", tree_flag ) ) {
        ImGui::PushItemWidth( 150.0f );
        ImGui::InputText( "InputText", &m_input_str );
        ImGui::InputDouble( "InputDouble", &m_input_double );
        ImGui::InputFloat( "InputFloat", &m_input_float );
        ImGui::SameLine( );
        ImGui::Dummy( { 20.0f, 5.0f } );
        ImGui::SameLine( );
        ImGui::SliderFloat( "SliderFloat", &m_slider_step_f, m_slider_min_f, m_slider_max_f );
        ImGui::InputInt( "InputInt", &m_input_int );
        ImGui::SameLine( );
        ImGui::Dummy( { 20.0f, 5.0f } );
        ImGui::SameLine( );
        ImGui::SliderInt( "SliderInt", &m_slider_step_i, m_slider_min_i, m_slider_max_i );
        ImGui::PopItemWidth( );
        ImGui::TreePop( );
    }

    if ( ImGui::TreeNodeEx( "Selection", tree_flag ) ) {
        ImGui::Checkbox( "Checkbox", &m_checkbox_toggle );

        ImGui::SetNextItemWidth( 200.0f );
        if ( ImGui::BeginCombo( "Combo", m_selected_fruit.c_str( ) ) ) {
            for ( const auto& fruit : m_fruits ) {
                if ( ImGui::Selectable( fruit.c_str( ), fruit == m_selected_fruit ) ) {
                    m_selected_fruit = fruit;
                }
            }
            ImGui::EndCombo( );
        }

        ImGui::SetNextItemWidth( 200.0f );
        if ( ImGui::BeginListBox( "Listbox" ) ) {
            for ( const auto& meat : m_meats ) {
                if ( ImGui::Selectable( meat.c_str( ), meat == m_selected_meat ) ) {
                    m_selected_meat = meat;
                }
            }
            ImGui::EndListBox( );
        }
        ImGui::SameLine( );
        ImGui::Text( "| Selected: %s", m_selected_meat.c_str( ) );

        if ( ImGui::RadioButton( "RadioButton", m_radio_toggle ) ) {
            m_radio_toggle = !m_radio_toggle;
        }
        ImGui::SameLine( );
        ImGui::Text( "| Status: %b", m_radio_toggle );
        ImGui::TreePop( );
    }

    if ( ImGui::TreeNodeEx( "Color && Progress", tree_flag ) ) {
        ImGui::ColorEdit4( "Color Picker", m_color );

        ImGui::ProgressBar( fmod( ImGui::GetTime( ) * 0.3f, 1.0f ), { 300.f, 0.f } );
        ImGui::TreePop( );
    }
}

void CDashboardView::on_exit( ) {}
CDashboardView::~CDashboardView( ) {}
