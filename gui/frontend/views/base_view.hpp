#pragma once
#include <config/config.hpp>

class CBaseView {
    public:
        virtual ~CBaseView( ) = default;
        virtual void render( ) = 0;
        virtual void on_enter( ) {}
        virtual void on_exit( ) {}

        struct ViewItem {
                const char* icon;
                const char* label;
                CBaseView* view;
        };
};
