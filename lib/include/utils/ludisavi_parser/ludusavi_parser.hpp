#pragma once
#include <types.hpp>
#include <yaml-cpp/yaml.h>

class CLudusaviParser {
    public:
        CLudusaviParser( );
        ~CLudusaviParser( );

        std::vector<ManifestSavePath> get_save_paths( uint32_t appid );
        fs::path                      resolve_path( std::string_view raw );

    private:
        fs::path m_path = paths::cache_dir( ) / "manifest.yaml";

        bool m_is_outdated     = false;
        bool m_manifest_exists = fs::exists( m_path );

        YAML::Node  m_manifest;
        std::string m_manifest_str = { };
        std::string m_manifest_link =
            "https://raw.githubusercontent.com/mtkennerly/ludusavi-manifest/master/data/manifest.yaml";

        std::unordered_map<uint32_t, YAML::Node> m_index;
};
