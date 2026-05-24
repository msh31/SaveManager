#pragma once

namespace ConfirmDialog {
    void render( );
    void show( std::string str, std::function<void( )> callback );

    inline bool                   is_open = false;
    inline std::function<void( )> fun;
    inline std::string            message = { };
}; // namespace ConfirmDialog
