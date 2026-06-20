#pragma once
#include <config/config.hpp>
#include <frontend/views/base_view.hpp>

#include <utils/blacklist/blacklist.hpp>

class CSettingsView : public CBaseView {
    public:
        CSettingsView( CConfig& cfg, Blacklist& blacklist );
        ~CSettingsView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        CConfig& m_config;
        Blacklist& m_blacklist;

        std::string m_blacklist_input;
        std::string m_new_game_name;
        std::string m_new_game_path;
        std::string m_new_game_appid;

        std::future<bool> m_update_future;
        std::future<std::pair<bool, bool>> m_update_t_future;
};
