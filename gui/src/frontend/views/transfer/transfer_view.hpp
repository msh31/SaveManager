#pragma once
#include <config/config.hpp>
#include <detection/detection.hpp>
#include <frontend/views/base_view.hpp>
#include <remote_transfer/remote_transfer.hpp>

#include <backend/detection_service/detection_service.hpp>

class CTransferView : public CBaseView {
    public:
        CTransferView( CConfig& config, CDetectionService& detection )
            : m_config( config ), m_detection( detection ) {};
        ~CTransferView( ) override = default;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override {}

    private:
        CConfig& m_config;
        CDetectionService& m_detection;
        uint64_t m_seen_generation = 0;

        std::vector<Game> m_games_snapshot;

        bool m_initialized = false;
        bool m_connected = false;
        bool m_was_transferring = false;
        bool m_use_password_auth = true;

        std::string m_dest_addr = { };
        std::string m_username = { };
        std::string m_password = { };
        std::string m_pubkey = { };
        std::string m_privkey = { };
        std::string m_key_passphrase = { };
        std::string m_current_remote_path{ };

        std::vector<RemoteEntry> m_remote_entries = { };

        std::unique_ptr<CRemoteTransfer> m_remote;

        std::future<void> m_future;
        std::future<bool> m_connect_future;

        std::atomic<int> m_current_file_index = 0;
        std::atomic<int> m_total_files = 0;
        int m_selected_remote_idx = -1;

        int m_selected_game_idx = 0;
        std::vector<fs::path> m_backups;
        std::vector<bool> m_selected_backups;
};
