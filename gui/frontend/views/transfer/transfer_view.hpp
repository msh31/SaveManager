#pragma once
#include <config/config.hpp>
#include <detection/detection.hpp>
#include <frontend/views/base_view.hpp>
#include <remote_transfer/remote_transfer.hpp>

#include <async_queue/async_queue.hpp>
#include <detection/detection_service.hpp>

class CTransferView : public CBaseView {
    public:
        ~CTransferView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override {}

    private:
        uint64_t m_seen_generation = 0;

        std::vector<Game> m_games_snapshot = { };

        bool m_initialized = false;
        bool m_connected = false;
        bool m_use_password_auth = true;

        std::string m_dest_addr = { };
        std::string m_username = { };
        std::string m_password = { };
        std::string m_pubkey = { };
        std::string m_privkey = { };
        std::string m_key_passphrase = { };
        std::string m_current_remote_path{ };

        std::vector<RemoteEntry> m_remote_entries = { };

        // shared_ptr: an in-flight upload/download can outlive this view once m_queue.shutdown() detaches it
        std::shared_ptr<CRemoteTransfer> m_remote;

        bool m_connecting = false;
        bool m_downloading = false;
        std::optional<TaskHandle> m_transfer_handle;

        int m_selected_remote_idx = -1;

        int m_selected_game_idx = 0;
        std::vector<fs::path> m_backups;
        std::vector<bool> m_selected_backups;

        CAsyncQueue m_queue;
};
