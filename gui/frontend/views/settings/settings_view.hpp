#pragma once
#include <frontend/views/base_view.hpp>

class CSettingsView : public CBaseView {
    public:
        ~CSettingsView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        std::vector<std::string> m_backgrounds = { };
        int m_current_background = 0;
};
