#pragma once

namespace Spinner {
    inline void render( ) {
        ImVec2 avail = ImGui::GetContentRegionAvail( );
        ImVec2 center( ( avail.x - 24 ) * 0.5f, ( avail.y - 40 ) * 0.5f );

        ImGui::BeginChild( "LoadingScreen", avail ); //, true, ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPos( center );

        static float angle = 0.0f;
        angle += ImGui::GetIO( ).DeltaTime * 5.0f;

        ImVec2 spinner_center = ImGui::GetCursorScreenPos( ) + ImVec2( 12, 12 );
        ImDrawList* draw_list = ImGui::GetWindowDrawList( );
        const int segments = 8;
        const float radius = 10.0f;
        const ImU32 base_color = ImGui::GetColorU32( ImGuiCol_Text );

        for ( int i = 0; i < segments; i++ ) {
            float a = angle + ( i * 2.0f * 3.14159f / segments );
            ImVec2 point( spinner_center.x + cosf( a ) * radius, spinner_center.y + sinf( a ) * radius );
            float alpha = ( i + 1.0f ) / segments;
            ImU32 segment_color = IM_COL32( 200, 200, 200, (int)( 255 * alpha ) );
            draw_list->AddCircleFilled( point, 3, segment_color );
        }

        ImGui::SetCursorPos( ImVec2( ( avail.x - ImGui::CalcTextSize( "Loading" ).x ) * 0.5f, center.y + 35 ) );
        ImGui::Text( "Loading" );
        ImGui::EndChild( );
    }
} // namespace Spinner
