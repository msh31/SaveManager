#pragma once
#include "backend/detection/detection.hpp"
#include "backend/features/remote_transfer/remote_transfer.hpp"

class Config;

struct TransferTab {
    void render(const Fonts& fonts, const Detection::DetectionResult& result, Config& config, TabState& state);

    bool initialized = false;
    bool connected = false;
    std::string dest_addr;
    std::string username;
    std::string password;
    std::string pubkey;
    std::string privkey;
    std::string key_passphrase;

    std::string current_remote_path;
    std::vector<RemoteEntry> remote_entries;
    int selected_remote_idx = -1;

    bool use_password_auth = true;
    std::unique_ptr<RemoteTransfer> remote;
    std::future<void> future;
    std::future<bool> connect_future;
    std::atomic<int> current_file_index = 0;
    std::atomic<int> total_files = 0;

    int spinner_frame = 0;
    const char* spinner = "|/-\\";
private:
};
