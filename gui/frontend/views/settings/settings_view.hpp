#pragma once
#include <frontend/views/base_view.hpp>
#include <async_queue/async_queue.hpp>

constexpr std::string_view ubi_translation_url =
    "https://raw.githubusercontent.com/msh31/SaveManager/refs/heads/dev/data/ubi_translations.json";
constexpr std::string_view pcgw_translation_url =
    "https://raw.githubusercontent.com/msh31/savemanager-manifest/refs/heads/main/data/manifest.json";

class CSettingsView : public CBaseView {
    public:
        ~CSettingsView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        CAsyncQueue m_queue;
        std::optional<TaskHandle> m_update_handle;
        std::optional<TaskHandle> m_ubi_translations_handle;
        std::optional<TaskHandle> m_manifest_handle;

        std::string m_blacklist_input = { };

        std::vector<std::string> m_backgrounds = { };
        int m_current_background = 0;
};
