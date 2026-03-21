#pragma once
#include "core/globals.hpp"
#include "core/detection/detection.hpp"
#include "core/helpers/remote_transfer/remote_transfer.hpp"

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

    std::string current_remote_path;
    std::vector<RemoteEntry> remote_entries;
    int selected_remote_idx = -1;

    std::unique_ptr<RemoteTransfer> remote;
    std::future<void> future;
    std::future<bool> connect_future;
    std::atomic<int> current_file_index = 0;
    std::atomic<int> total_files = 0;
};
