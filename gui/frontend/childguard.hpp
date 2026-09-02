#pragma once

class ChildGuard {
    public:
        ChildGuard(
            const char* str, const ImVec2 size,
            ImGuiChildFlags child_flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY |
                                          ImGuiChildFlags_AutoResizeX,
            ImGuiWindowFlags window_flags = 0 ) {
            ImGui::BeginChild( str, size, child_flags, window_flags );
        }
        ~ChildGuard( ) { ImGui::EndChild( ); }

        ChildGuard( const ChildGuard& ) = delete;
        ChildGuard& operator=( const ChildGuard& ) = delete;
        ChildGuard( ChildGuard&& ) = delete;
        ChildGuard& operator=( ChildGuard&& ) = delete;
};