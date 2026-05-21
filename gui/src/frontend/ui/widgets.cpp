#include "widgets.hpp"

namespace Widgets {
bool begin_game_card( const char *id, const Fonts &fonts, bool &expanded, const char *title, const char *right_text ) {
    const char *chevron = expanded ? "▼" : "▶";

    ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 198 / 255.f, 97 / 255.f, 63 / 255.f, 1.f ) );
    ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 4.0f );
    ImGui::BeginChild( id, ImVec2( 0, 0 ), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
    ImGui::PopStyleColor( );

    if ( ImGui::Selectable( "##header", false, ImGuiSelectableFlags_None, ImVec2( 0, 30 ) ) ) expanded = !expanded;
    ImGui::SameLine( 8.0f );

    ImGui::PushFont( fonts.bold );
    ImGui::TextColored( ImColor( 198, 97, 63 ).Value, "%s", chevron );
    ImGui::PopFont( );
    ImGui::SameLine( );

    ImGui::PushFont( fonts.medium );
    ImGui::Text( "%s", title );
    ImGui::PopFont( );

    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - ImGui::CalcTextSize( right_text ).x );
    ImGui::Text( "%s", right_text );

    return expanded;
}

void end_game_card( ) {
    ImGui::EndChild( );
    ImGui::PopStyleVar( );
}

} // namespace Widgets
