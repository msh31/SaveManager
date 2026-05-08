#include "dashboard.hpp"
#include "constants.hpp"
#include "types.hpp"
#include "backend/features/backup/backup.hpp"

#include "frontend/ui/notifications/notification.hpp"
#include <frontend/views/backups/backup_view.hpp>

#ifdef __APPLE__
#include <spawn.h>
#include <sys/wait.h>
#endif

static constexpr const char* spinner = "|/-\\";
static float button_spacing = 4.0f;
static float btn_width = 80.0f;

static constexpr const char* ICON_SORT   = "\xef\x83\x9c";
static constexpr const char* ICON_FILTER = "\xef\x82\xb0";

void DashboardTab::on_result_changed(RenderContext& ctx) {
    std::shared_lock<std::shared_mutex> lock(ctx.result.d_mutex);
    grouped_games = {};
    game_cache.clear();

    grouped_games = ctx.result.get_grouped();
    last_game_count = ctx.games.size();

    try {
        for (const auto& game : ctx.games) {
            GameCache cache;
            if (!fs::is_directory(game.save_path)) continue;

            if(game.type != PlatformType::MINECRAFT) {
                for (const auto& file : fs::recursive_directory_iterator(game.save_path, fs::directory_options::skip_permission_denied)) {
                    if (!fs::is_regular_file(file)) continue;
                    auto ext = file.path().extension().string();
                    if(game.type != PlatformType::CUSTOM) {
                        if(std::find(extension_blocklist.begin(), extension_blocklist.end(), ext) != extension_blocklist.end()) continue;
                    }
                    if(std::find(g_extension_blocklist.begin(), g_extension_blocklist.end(), ext) != g_extension_blocklist.end()) continue;
                    cache.save_files.push_back(file.path());
                }
            } else {
                cache.save_files.push_back(game.save_path);
            }
            cache.backup_count = Features::get_backups(game.game_name, ctx.config).size();
            cache.labels = Features::load_labels(game.game_name, ctx.config);
            auto key = cache_key(game);
            if(game.type == PlatformType::MINECRAFT) {
                if(game_cache.contains(key)) {
                    game_cache[key].save_files.push_back(game.save_path);
                } else {
                    game_cache[cache_key(game)] = cache;
                }
            } else {
                game_cache[cache_key(game)] = cache;
            }
        }

        for (const auto& entry : ctx.games) {
            fs::file_time_type current_max;
            if (!fs::is_directory(entry.save_path)) continue;
            // get_logger().info("checking path: {}", entry.save_path.string());
            for (const auto& file : fs::directory_iterator(entry.save_path, fs::directory_options::skip_permission_denied)) {
                if(!fs::exists(file)) continue;
                auto t = fs::last_write_time(file);
                if (fs::is_regular_file(file)) if (t > current_max) current_max = t;
            }
            game_last_modified.insert({ entry.game_name, current_max });
        }

    }
    catch (fs::filesystem_error& er) {
        get_logger().error("dashboard(on_result_changed): {}", er.what());
    }
}

void DashboardTab::render(const Fonts& fonts, Detection::DetectionResult& result, Config& config) { 
    static BackupTab backup;

    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_DrawSelectedOverline)) {
        if (ImGui::BeginTabItem("Games")) {
            std::vector<Game> snapshot;
            {
                std::shared_lock<std::shared_mutex> lock(result.d_mutex);
                snapshot = result.games;
            }

            RenderContext ctx{result,config, fonts, snapshot};
            spinner_frame++;

            if (refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                Notify::show_notification("Save refresh", "Saves refreshed!", 2000);
                refresh_future.get();
                on_result_changed(ctx);
            }

            if(last_game_count != snapshot.size()) {
                on_result_changed(ctx);
            }

            render_toolbar(ctx);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            render_game_list(ctx);
            ImGui::PopStyleVar();
            render_modals(ctx);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Backups"))  {
            backup.render(fonts, result, config);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void DashboardTab::render_toolbar(RenderContext& ctx) {
    ImGui::PushFont(ctx.fonts.header);
    ImGui::Text("Dashboard");
    ImGui::PopFont();

    bool is_refreshing = refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
    bool is_backing_up = backup_future.valid() && backup_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;

    float sort_width = ImGui::CalcTextSize("Sort: Alphabetical").x + ImGui::GetStyle().FramePadding.x * 2;
    float filter_width = ImGui::CalcTextSize("Filter: Rockstar").x + ImGui::GetStyle().FramePadding.x * 2;
    float refresh_width = ImGui::CalcTextSize("Refresh").x + ImGui::GetStyle().FramePadding.x * 2;
    float backup_width = ImGui::CalcTextSize("Mass Backup").x + ImGui::GetStyle().FramePadding.x * 2;
    float spacing = ImGui::GetStyle().ItemSpacing.x * 3;

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - sort_width - refresh_width - backup_width - filter_width - spacing);

    if((ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F))) {
        focus_search = true;
    }
    if(focus_search) {
        ImGui::SetKeyboardFocusHere();
        focus_search = false;
    }
    ImGui::InputText("##search", &search_query);
    ImGui::SameLine();

    std::string sort_label = sort_mode == SortMode::Alphabetical
        ? std::format("{} A-Z", ICON_SORT)
        : std::format("{} Date", ICON_SORT);

    if (ImGui::Button(sort_label.c_str())) {
        sort_mode = sort_mode == SortMode::Alphabetical ? SortMode::Recent : SortMode::Alphabetical;
    }
    ImGui::SameLine();
    std::string filter_label = platform_filter.has_value() ? std::format("{} {}", ICON_FILTER, get_platform_label(*platform_filter)) : std::format("{} All", ICON_FILTER);
    if (ImGui::Button(filter_label.c_str())) {
        if (!platform_filter.has_value()) {
            platform_filter = PlatformType::UBISOFT;
        } else {
            switch (*platform_filter) {
                case PlatformType::UBISOFT:  platform_filter = PlatformType::ROCKSTAR; break;
                case PlatformType::ROCKSTAR: platform_filter = PlatformType::UNREAL;   break;
                case PlatformType::UNREAL:   platform_filter = PlatformType::CUSTOM;   break;
                case PlatformType::CUSTOM:   platform_filter = std::nullopt;            break;
                default:                     platform_filter = std::nullopt;            break;
            }
        }
    }
    ImGui::SameLine();

    if(!is_refreshing && (ImGui::Button("Refresh") || (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R)))) {
        {
            std::unique_lock lock(ctx.result.d_mutex);
            ctx.result.games.clear();
            grouped_games.clear();
        }
        refresh_future = std::async(std::launch::async, [&result = ctx.result, &config = ctx.config]() { Detection::find_saves(config, result); });
    }
    ImGui::SetItemTooltip("Re-runs the detection logic to find new saves");
    ImGui::SameLine();
    if(!is_backing_up) {
        if(ImGui::Button("Mass Backup")) {
            backup_future = std::async(std::launch::async, [&result = ctx.result, &config = ctx.config]() {
                std::vector<Game> snapshot;
                {
                    std::shared_lock lock(result.d_mutex);
                    snapshot = result.games;
                }
                for (auto& entry : snapshot) {
                    if(fs::is_directory(entry.save_path)) {
                        for (const auto& file : fs::recursive_directory_iterator(entry.save_path, fs::directory_options::skip_permission_denied)) {
                            if (fs::is_regular_file(file)) {
                                auto ext = file.path().extension().string();
                                if(std::find(extension_blocklist.begin(), extension_blocklist.end(), ext) != extension_blocklist.end()) continue;
                                Features::backup_game(entry, file.path(), config);
                            }
                        }
                    }
                }
            });
        }
        ImGui::SetItemTooltip("Creates a backup of all games found!");
    } else {
        char spin_char = spinner[(spinner_frame / 10) % 4];
        std::string loading_text = std::string("Creating backups...") + spin_char;
        ImGui::Text("%s", loading_text.c_str());
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
}

void DashboardTab::render_game_list(RenderContext& ctx) {
    // std::lock_guard<std::mutex> lock(ctx.result.d_mutex);
    std::transform(search_query.begin(), search_query.end(), search_query.begin(),
                   ::tolower);

    auto sorted = grouped_games;
    switch (sort_mode) {
        case SortMode::Recent:
            std::sort(sorted.begin(), sorted.end(), [&](const std::vector<int>& a, const std::vector<int>& b) {
                return game_last_modified[ctx.games[a[0]].game_name] >
                game_last_modified[ctx.games[b[0]].game_name];
            });
        break;
        case SortMode::Alphabetical:
            std::sort(sorted.begin(), sorted.end(), [&](const std::vector<int>& a, const std::vector<int>& b) {
                return ctx.games[a[0]].game_name < ctx.games[b[0]].game_name;
            });
        break;
    }


    enumerate(sorted, [&](int gi, auto& group) {
        if(platform_filter.has_value() && ctx.games[group[0]].type != *platform_filter) return;

        const Game& primary = ctx.games[group[0]];
        std::string game_name = primary.game_name;

        // filter
        std::transform(game_name.begin(), game_name.end(), game_name.begin(),
                       ::tolower);
        if (!search_query.empty()) {
            if(game_name.find(search_query) == std::string::npos) {
                return;
            }
        }
        render_game_row(ctx, group, static_cast<int>(gi));
        ImGui::Dummy(ImVec2(0, 6.0f));
    });
}

void DashboardTab::render_game_row(RenderContext& ctx, const std::vector<int>& group, int gi) {
    const Game& primary = ctx.games[group[0]];

    const float card_padding = 8.0f;
    auto top = ImGui::GetCursorScreenPos();
    bool& not_collapsed = card_collapsed[cache_key(primary)]; //defaults to false
    bool& bk_collapsed = backups_collapsed[cache_key(primary)];

    std::vector<std::pair<fs::path, const Game*>> files = {};
    auto& cache = game_cache[cache_key(primary)];
    int save_count = cache.save_files.size();
    int backup_count = cache.backup_count;
    auto labels = cache.labels;

    for (const auto& path : cache.save_files) {
        files.emplace_back(path, &primary);
    }

    const char* chevron = not_collapsed ? "▼" : "▶";
    const char* chevron_b = bk_collapsed ? "▶" : "▼";
    auto selectable_id = std::format("##gamename_{}", gi);

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(198/255.f, 97/255.f, 63/255.f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
    ImGui::BeginChild(selectable_id.c_str(), ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
    ImGui::PopStyleColor();

    if(ImGui::Selectable("##header", false, ImGuiSelectableFlags_None, ImVec2(0, 30))) {
        not_collapsed = !not_collapsed;
    }
    ImGui::SameLine(8.0f);

    ImGui::PushFont(ctx.fonts.bold);
    ImGui::TextColored(ImColor(198, 97, 63).Value, "%s", chevron);
    ImGui::PopFont();
    ImGui::SameLine();

    ImGui::PushFont(ctx.fonts.medium);
    std::string left_text = std::format("{}", primary.game_name);
    ImGui::Text("%s", left_text.c_str());
    ImGui::PopFont();
    std::string right_text = std::format("{} | {} saves | {} backups", get_platform_label(primary.type),
                                         save_count, backup_count);
    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(right_text.c_str()).x);
    ImGui::Text("%s", right_text.c_str());

    if (not_collapsed) {
        if(save_count > 0) {
            if(primary.type != PlatformType::MINECRAFT) ImGui::TextDisabled("SAVE FILES");
            else ImGui::TextDisabled("WORLDS");

            static std::string test_str = "-------------";
            ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(test_str.c_str()).x);

            ImGui::PushStyleColor(ImGuiCol_Button, ImColor(198, 97, 63).Value);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(198, 97, 63).Value);
            if(ImGui::Button("Backup All")) {
                backup_future = std::async(std::launch::async, [this, primary, ctx, files]() {
                get_logger().info("creating backup of: {}", primary.game_name);

                fs::path game_backup_dir = ctx.config.settings.backup_path / sanitize_filename(primary.game_name);
                if(!fs::exists(game_backup_dir)) fs::create_directories(game_backup_dir);

                fs::path final_path = game_backup_dir / Features::construct_backup_name(primary.game_name);
                fs::path zip_name = final_path.parent_path() / (final_path.filename().string() + ".tmp");

                ZipArchive za(MODE_CREATE_ARCHIVE, zip_name);
                bool failed_to_add = false;
                for(const auto& entry : files) { 
                    if(!za.add_to_archive(entry.first)) {
                        failed_to_add = true;
                    }
                }
                if(failed_to_add) {
                    fs::remove(zip_name);
                    Notify::show_notification("Backup Creation", "Failed to create backup! Please refer to the logfile!", 2000);
                } else { 
                    fs::rename(zip_name, final_path);
                    Notify::show_notification("Backup Created", "A backup has been for all saves!", 1500);
                }
                });
            }
            ImGui::PopStyleColor(2);

            for (auto& save : files) {
                ImGui::Separator();
                render_save_row(ctx, save.first, *save.second);
                ImGui::Separator();
            }
        } else ImGui::TextDisabled("Game detected but no saves were found!");
        if(backup_count > 0) {
            if(ImGui::Selectable("##backups", false, ImGuiSelectableFlags_None, ImVec2(0, 30))) {
                bk_collapsed = !bk_collapsed;
            }
            ImGui::SameLine(8.0f);

            ImGui::PushFont(ctx.fonts.bold);
            ImGui::TextColored(ImColor(198, 97, 63).Value, "%s", chevron_b);
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::PushFont(ctx.fonts.medium);
            ImGui::Text("BACKUPS");
            ImGui::PopFont();

            if (!bk_collapsed) {
                auto backups = Features::get_backups(primary.game_name, ctx.config);
                for (auto& backup : backups) {
                    ImGui::Separator();
                    render_backup_row(ctx, backup, primary, labels);
                    ImGui::Separator();
                }
            }
        }
    }

    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Open Path")) {
#ifdef __linux__
            pid_t pid = fork();
            if (pid == 0) {
                execl("/usr/bin/xdg-open", "xdg-open", primary.save_path.string().c_str(), nullptr);
                _exit(1);
            }
#endif
#ifdef _WIN32
            ShellExecuteA(NULL, "open", primary.save_path.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
#ifdef __APPLE__
            extern char **environ;
            pid_t pid;
            std::string path = primary.save_path.string();

            const char* argv[] = { "open", path.c_str(), nullptr };
            int status = posix_spawn(&pid, "/usr/bin/open", nullptr, nullptr, (char* const*)argv, environ);
            if (status == 0) {
                waitpid(pid, &status, 0);
            }
#endif
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void DashboardTab::render_save_row(RenderContext& ctx, const fs::path& save_file, const Game& game) {
    ImGui::PushID(save_file.string().c_str());
    if(!fs::exists(save_file)) { ImGui::PopID(); return; }

    std::string date_text = std::format("{:%d/%m/%y %H:%M} | ", fs::last_write_time(save_file));
    float date_width = ImGui::CalcTextSize(date_text.c_str()).x;

    std::string size_text;
    if(game.type != PlatformType::MINECRAFT) {
        auto b_size = fs::file_size(save_file) / 1024;
        size_text = std::format("{}KB  ", b_size);
    } 

    float size_width = ImGui::CalcTextSize(size_text.c_str()).x;
    float total_width = date_width + size_width + btn_width * 1 + button_spacing * 5;

    if(game.show_parent_path) {
        ImGui::Text("%s", (save_file.parent_path().filename() / save_file.filename()).string().c_str());
    } else {
        ImGui::Text("%s", save_file.filename().string().c_str());
    }

    ImGui::SameLine(ImGui::GetContentRegionMax().x - total_width);

    ImGui::TextDisabled("%s", date_text.c_str());
    ImGui::SameLine(0.0f, button_spacing);
    ImGui::TextDisabled("%s", size_text.c_str());
    ImGui::SameLine(0.0f, button_spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 3.0f));
    if(ImGui::Button("Backup", ImVec2(btn_width, 0))) {
        backup_future = std::async(std::launch::async, [game, save_file, &config = ctx.config]() {
                Features::backup_game(game, save_file, config);
                });
    }
    ImGui::SetItemTooltip("Create a backup of this save");
    ImGui::PopStyleVar();
    ImGui::PopID();
}

void DashboardTab::render_backup_row(RenderContext& ctx, const fs::path& backup, const Game& game, const std::unordered_map<std::string, std::string>& labels) {
    ImGui::PushID(backup.string().c_str());
    if(!fs::exists(backup)) { ImGui::PopID(); return; }

    auto it = labels.find(backup.filename().string());
    std::string display = (it != labels.end()) ? it->second : backup.filename().string();

    std::string date_text = std::format("{:%d/%m/%y %H:%M} | ", fs::last_write_time(backup));
    float date_width = ImGui::CalcTextSize(date_text.c_str()).x;
    auto b_size = fs::file_size(backup) / 1024;

    std::string size_text = std::format("{}KB  ", b_size);
    float size_width = ImGui::CalcTextSize(size_text.c_str()).x;

    float total_width = date_width + size_width + btn_width * 3 + button_spacing * 5;

    ImGui::Text("%s", display.c_str());
    ImGui::SameLine(ImGui::GetContentRegionMax().x - total_width);

    ImGui::TextDisabled("%s", date_text.c_str());
    ImGui::SameLine(0.0f, button_spacing);
    ImGui::TextDisabled("%s", size_text.c_str());
    ImGui::SameLine(0.0f, button_spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 3.0f));
    if(ImGui::Button("Restore", ImVec2(btn_width, 0))) {
        Features::restore_backup(backup, game.save_path);
    }
    ImGui::SetItemTooltip("Restore save from backup");
    ImGui::SameLine(0.0f, button_spacing);

    if(ImGui::Button("Rename", ImVec2(btn_width, 0))) {
        pending_rename_game = game;
        pending_rename_backup = backup;
        rename_input = (it != labels.end()) ? it->second : "";
        open_rename_modal = true;
    }
    ImGui::SetItemTooltip("Rename this backup");
    ImGui::SameLine(0.0f, button_spacing);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    if(ImGui::Button("Delete", ImVec2(btn_width, 0))) {
        if(fs::remove(backup)) {
            auto mutable_labels = labels;
            mutable_labels.erase(backup.filename().string());
            Features::save_labels(game.game_name, ctx.config, mutable_labels);
            Notify::show_notification("Backup Deletion", "Backup deleted!", 1500);
        } else {
            Notify::show_notification("Backup Deletion", "Backup could not be deleted!", 1500);
        }
    }
    ImGui::SetItemTooltip("Delete backed up savegame");

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::PopID();
}

void DashboardTab::render_modals(RenderContext& ctx) {
    if (open_rename_modal) {
        open_rename_modal = false;
        ImGui::OpenPopup("Rename Backup");
    }

    if (ImGui::BeginPopupModal("Rename Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", pending_rename_backup.filename().string().c_str());
        ImGui::InputText("Label", &rename_input);
        if (ImGui::Button("Save")) {
            Features::save_label(pending_rename_game.game_name, ctx.config,
                                 pending_rename_backup.filename().string(), rename_input);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
