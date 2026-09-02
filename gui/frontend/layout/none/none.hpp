#pragma once
#include <frontend/layout/shell.hpp>

class CNoShell : public IShell {
    public:
        CBaseView* render( CBaseView* active ) override;
};
