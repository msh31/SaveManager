#pragma once

class CBaseView {
    public:
        virtual ~CBaseView( )    = default;
        virtual void render( )   = 0;
        virtual void on_enter( ) = 0;
        virtual void on_exit( )  = 0;

        struct ViewItem {
                const char* icon;
                const char* label;
                CBaseView*  view;
        };
};
