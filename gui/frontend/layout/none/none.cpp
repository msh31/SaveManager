#include "none.hpp"

#include <frontend/childguard.hpp>

CBaseView* CNoShell::render( CBaseView* active ) {
    ChildGuard content_child( "##content", { 0, 0 }, ImGuiChildFlags_Borders );
    if ( active ) active->render( );

    return nullptr;
}
