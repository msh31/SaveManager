#pragma once
#include <config/config.hpp>
#include <frontend/views/base_view.hpp>

class CSettingsView : public CBaseView {
    public:
        CSettingsView( CConfig& cfg );
        ~CSettingsView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        CConfig& m_config;
};
