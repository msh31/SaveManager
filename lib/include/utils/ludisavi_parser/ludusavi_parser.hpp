#pragma once
#include <types.hpp>

class CLudusaviParser {
    public:
        struct ResolveContext {
                fs::path install_dir;
                fs::path proton_prefix;
        };

        CLudusaviParser( );
        ~CLudusaviParser( );

        std::vector<ManifestSavePath> get_save_paths( uint32_t appid, const ResolveContext& ctx );

    private:
        fs::path resolve_path(
            std::string_view raw, const ManifestSavePath& entry, const ResolveContext& ctx, uint32_t appid ) const;

        fs::path m_path             = paths::cache_dir( ) / "manifest.yaml";
        fs::path m_index_cache_path = paths::cache_dir( ) / "manifest_index.json";

        bool m_is_outdated     = false;
        bool m_manifest_exists = fs::exists( m_path.string( ) );

        std::string m_manifest_link =
            "https://raw.githubusercontent.com/mtkennerly/ludusavi-manifest/master/data/manifest.yaml";

        std::string m_home;
        std::string m_xdg_data;
        std::string m_xdg_config;
        std::string m_store_user_id;
        std::string m_os_user_name;

        // appid -> pre-extracted "save"-tagged entries (templates, unresolved).
        std::unordered_map<uint32_t, std::vector<ManifestSavePath>> m_index;
};
