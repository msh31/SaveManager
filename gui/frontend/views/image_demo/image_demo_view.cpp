#include "image_demo_view.hpp"
#include <logger.hpp>

#include <frontend/childguard.hpp>
#include <frontend/images/pepe.hpp>
#include <frontend/ui.hpp>

void CImageDemoView::on_enter( ) {
    m_embedded = CImageManager::get( ).load_from_memory( pepe_png, pepe_png_len, "pepe" );
};

void CImageDemoView::render( ) {
    ImGui::Text( "Hello, this is the image demo view" );

    auto avail = ImGui::GetContentRegionAvail( );
    ImVec2 child_size( ( avail.x - ImGui::GetStyle( ).ItemSpacing.x ) * 0.5f, avail.y );

    {
        ChildGuard cover( "pepe_cover", child_size, ImGuiChildFlags_Borders );
        ImGui::TextDisabled( "cover" );

        auto p_min = ImGui::GetCursorScreenPos( );
        auto size = ImGui::GetContentRegionAvail( );

        ui::add_cover_image(
            ImGui::GetWindowDrawList( ), (ImTextureID)m_embedded.texture_id, p_min,
            ImVec2( p_min.x + size.x, p_min.y + size.y ), (float)m_embedded.texture_width,
            (float)m_embedded.texture_height );

        ImGui::Dummy( size );
    }

    ImGui::SameLine( );

    {
        ChildGuard contain( "pepe_contain", child_size, ImGuiChildFlags_Borders );
        ImGui::TextDisabled( "contain" );

        auto p_min = ImGui::GetCursorScreenPos( );
        auto size = ImGui::GetContentRegionAvail( );

        ui::add_contain_image(
            ImGui::GetWindowDrawList( ), (ImTextureID)m_embedded.texture_id, p_min,
            ImVec2( p_min.x + size.x, p_min.y + size.y ), (float)m_embedded.texture_width,
            (float)m_embedded.texture_height );

        ImGui::Dummy( size );
    }
}

void CImageDemoView::on_exit( ) { m_embedded = { }; }
