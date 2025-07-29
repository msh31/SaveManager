using Spectre.Console;
using SaveManager.Models;
using SaveManager.Managers;

namespace SaveManager.Utilities;

public class SaveManager(Globals globals, Utilities utilities, ConfigManager configManager, Logger logger)
{
    private const string DefaultDisplayName = "CUSTOM_NAME_NOT_SET";
    private const double BytesToKb = 1024.0;

    public async Task InitializeAsync()
    {
        var accountFolders = utilities.GetAccountId(globals, configManager);
        
        if (accountFolders.Count == 0) {
            AnsiConsole.MarkupLine("[red][[err]][/] No Ubisoft accounts found.");
            return;
        }

        foreach (var accountFolder in accountFolders) {
            var accountId = Path.GetFileName(accountFolder);
            AnsiConsole.MarkupLine($"[cyan][[inf]][/] Processing account: {accountId}");
            
            await ScanAccount(accountId);
        }
    }
    
    private async Task ScanAccount(string accountId)
    {
        await FindGames(accountId);
        await FindSaveFiles(accountId);
    }

    private async Task FindSaveFiles(string accountId)
    {
        AnsiConsole.MarkupLine("[cyan][[inf]][/] Looking for savegames..");
        
        var existingNames = await LoadExistingDisplayNames();
        var allSaves = new Dictionary<string, List<SaveFileInfo>>();

        foreach (var gameId in configManager.Data.DetectedUbiGames) {
            var saves = await ScanGameFolder(accountId, gameId, existingNames);

            if (!saves.Any()) {
                continue;
            }
            
            var saveKey = $"{accountId}_{gameId}";
            allSaves[saveKey] = saves;
            await DisplayGameSaves(gameId, saves);
        }

        await SaveToFile(allSaves);
    }

    private async Task<List<SaveFileInfo>> ScanGameFolder(string accountId, string gameId, Dictionary<string, Dictionary<string, Dictionary<string, string>>> existingNames)
    {
        var gameFolder = Path.Combine(globals.UbisoftRootFolder, accountId, gameId);

        if (!Directory.Exists(gameFolder)) {
            return new List<SaveFileInfo>();
        }

        var files = Directory.GetFiles(gameFolder);
        var validSaves = new List<SaveFileInfo>();

        foreach (var file in files) {
            if (IsValidSaveFile(file)) {
                var saveInfo = CreateSaveInfo(file, accountId, gameId, existingNames);
                
                validSaves.Add(saveInfo);
            }
        }

        return validSaves;
    }

    private bool IsValidSaveFile(string filePath)
    {
        var fileName = Path.GetFileName(filePath);

        if (fileName.Contains("[Options]", StringComparison.OrdinalIgnoreCase)) {
            return false;
        }

        if (!fileName.EndsWith(".save", StringComparison.OrdinalIgnoreCase)) {
            return false;
        }
    
        return true;
    }

    public async Task ListSaves(string platform)
    {
        var dataFilePath = platform.ToLower() switch {
            "ubisoft" => globals.UbiSaveInfoFilePath,
            _ => globals.UbiSaveInfoFilePath 
        };
        
        if (!File.Exists(dataFilePath)) {
            AnsiConsole.MarkupLine($"[red][[err]][/] No {platform} saves found.");
            return;
        }

        try {
            var json = await File.ReadAllTextAsync(dataFilePath);
            var allSaves = System.Text.Json.JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json, JsonContext.Default.DictionaryStringListSaveFileInfo);
            
            if (allSaves == null || allSaves.Count == 0) {
                AnsiConsole.MarkupLine($"[red][[err]][/] No {platform} saves found.");
                return;
            }

            AnsiConsole.MarkupLine($"[cyan][[inf]][/] Listing all {platform} saves:");

            foreach (var entry in allSaves) {
                string[] parts = entry.Key.Split('_');
                var accountId = parts[0];
                var gameId = parts[1];
                
                string gameName = gameId;
                
                if (platform.Equals("Ubisoft", StringComparison.OrdinalIgnoreCase)) {
                    gameName = await utilities.TranslateUbisoftGameId(Path.Combine(globals.UbisoftRootFolder, accountId, gameId));
                }

                AnsiConsole.Markup($"\n[darkcyan]{Markup.Escape(gameName)} - [/]");
                AnsiConsole.Markup($"[yellow]{gameId} - [/]");
                
                if (platform.Equals("Ubisoft", StringComparison.OrdinalIgnoreCase)) {
                    AnsiConsole.Markup($"[red]Account: {accountId} - [/]");
                }
                
                AnsiConsole.MarkupLine($"[white]Total Saves: {entry.Value.Count}[/]");

                foreach (var save in entry.Value) {
                    var sizeInKb = save.FileSizeBytes / BytesToKb;
                    var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                    var displayName = save.DisplayName == DefaultDisplayName ? save.FileName : save.DisplayName;
                    
                    AnsiConsole.MarkupLine($"[gray]   - {Markup.Escape(displayName)} | {sizeInKb:F1}KB | modified: {timestamp}[/]");
                }
            }
        } catch (Exception ex) {
            AnsiConsole.MarkupLine($"[red][[err]][/] Error loading {platform} save info: {ex.Message}");
        }
    }

    public async Task RenameSave(string platform, string gameId, string fileName, string newName)
    {
        var savesData = await LoadExistingDisplayNames();
        bool updated = false;

        foreach (var accountEntry in savesData)
        {
            if (accountEntry.Value.ContainsKey(gameId))
            {
                var gameData = accountEntry.Value[gameId];
                if (gameData.ContainsKey(fileName))
                {
                    var gameName = await utilities.TranslateUbisoftGameId(Path.Combine(globals.UbisoftRootFolder, accountEntry.Key, gameId));
                    string oldDisplayName = gameData[fileName] == DefaultDisplayName ? fileName : gameData[fileName];
                    gameData[fileName] = newName;
                    updated = true;
                    
                    AnsiConsole.MarkupLine($"[green][[suc]][/] Renamed save in [darkcyan]{Markup.Escape(gameName)}[/]");
                    AnsiConsole.MarkupLine($"[gray]   From: {Markup.Escape(oldDisplayName)}[/]");
                    AnsiConsole.MarkupLine($"[gray]   To: {Markup.Escape(newName)}[/]");
                    break;
                }
            }
        }

        if (updated) {
            await SaveDisplayNames(savesData);
        } else {
            AnsiConsole.MarkupLine($"[red][[err]][/] Save file '{fileName}' not found in game with ID '{gameId}'.");
        }
    }

    private async Task<Dictionary<string, Dictionary<string, Dictionary<string, string>>>> LoadExistingDisplayNames()
    {
        var result = new Dictionary<string, Dictionary<string, Dictionary<string, string>>>();
        var dataFile = Path.Combine(Path.GetDirectoryName(globals.ConfigFilePath) ?? "", globals.UbiSaveInfoFilePath);

        if (!File.Exists(dataFile)) {
            return result;
        }

        try {
            var json = await File.ReadAllTextAsync(dataFile);
            var allSaves = System.Text.Json.JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json, JsonContext.Default.DictionaryStringListSaveFileInfo);
            
            if (allSaves != null) {
                foreach (var saveGroup in allSaves) {
                    var parts = saveGroup.Key.Split('_', 2);
                    
                    if (parts.Length == 2) {
                        var accountId = parts[0];
                        var gameId = parts[1];

                        if (!result.ContainsKey(accountId)) {
                            result[accountId] = new Dictionary<string, Dictionary<string, string>>();
                        }

                        if (!result[accountId].ContainsKey(gameId)) {
                            result[accountId][gameId] = new Dictionary<string, string>();
                        }
                        
                        foreach (var save in saveGroup.Value) {
                            result[accountId][gameId][save.FileName] = save.DisplayName;
                        }
                    }
                }
            }
        } catch (Exception ex) {
            logger.Error($"Error loading display names: {ex.Message}");
        }

        return result;
    }

    private async Task DisplayGameSaves(string gameId, List<SaveFileInfo> saves)
    {
        if (!saves.Any()) {
            return;
        }

        var accountId = saves.First().AccountId;
        var gameName = await utilities.TranslateUbisoftGameId(Path.Combine(globals.UbisoftRootFolder, accountId, gameId));

        AnsiConsole.MarkupLine($"[red]{Markup.Escape(gameName)} - {saves.Count} Detected Saves[/]");

        foreach (var save in saves) {
            var sizeInKb = save.FileSizeBytes / 1024.0;
            var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
            var displayName = save.DisplayName == DefaultDisplayName ? save.FileName : save.DisplayName;
            var escapedDisplayName = Markup.Escape(displayName);
            
            AnsiConsole.MarkupLine($"[darkcyan]   - {escapedDisplayName} | {sizeInKb:F1}KB | created: {timestamp} | updated: {timestamp}[/]");
        }
    }

    private async Task SaveToFile(Dictionary<string, List<SaveFileInfo>> allSaves)
    {
        try {
            var dataFile = Path.Combine(Path.GetDirectoryName(globals.ConfigFilePath) ?? "", globals.UbiSaveInfoFilePath);
            var json = System.Text.Json.JsonSerializer.Serialize(allSaves, JsonContext.Default.DictionaryStringListSaveFileInfo);
            await File.WriteAllTextAsync(dataFile, json);
        } catch (Exception ex) {
            logger.Error($"Error saving to file: {ex.Message}");
            AnsiConsole.MarkupLine($"[red][[err]][/] Error saving data: {ex.Message}");
        }
    }

    private SaveFileInfo CreateSaveInfo(string filePath, string accountId, string gameId, Dictionary<string, Dictionary<string, Dictionary<string, string>>> existingNames)
    {
        var fileName = Path.GetFileName(filePath);
        var fileInfo = new FileInfo(filePath);
        
        var displayName = DefaultDisplayName;
        
        if (existingNames.ContainsKey(accountId) && existingNames[accountId].ContainsKey(gameId) && existingNames[accountId][gameId].ContainsKey(fileName)) {
            displayName = existingNames[accountId][gameId][fileName];
        }

        return new SaveFileInfo {
            FileName = fileName,
            FilePath = filePath,
            GameId = gameId,
            AccountId = accountId,
            DisplayName = displayName,
            FileSizeBytes = fileInfo.Length,
            LastModified = fileInfo.LastWriteTime
        };
    }

    private async Task SaveDisplayNames(Dictionary<string, Dictionary<string, Dictionary<string, string>>> savesData)
    {
        try {
            var dataFile = Path.Combine(Path.GetDirectoryName(globals.ConfigFilePath) ?? "", globals.UbiSaveInfoFilePath);
            var allSaves = new Dictionary<string, List<SaveFileInfo>>();

            foreach (var accountEntry in savesData) {
                foreach (var gameEntry in accountEntry.Value) {
                    var saveKey = $"{accountEntry.Key}_{gameEntry.Key}";
                    var saveInfoList = new List<SaveFileInfo>();

                    foreach (var saveEntry in gameEntry.Value) {
                        saveInfoList.Add(new SaveFileInfo {
                            FileName = saveEntry.Key,
                            DisplayName = saveEntry.Value,
                            AccountId = accountEntry.Key,
                            GameId = gameEntry.Key
                        });
                    }

                    allSaves[saveKey] = saveInfoList;
                }
            }

            var json = System.Text.Json.JsonSerializer.Serialize(allSaves, JsonContext.Default.DictionaryStringListSaveFileInfo);
            await File.WriteAllTextAsync(dataFile, json);
        } catch (Exception ex) {
            logger.Error($"Error saving display names: {ex.Message}");
        }
    }

    private async Task FindGames(string accountId)
    {
        var accountRootFolder = Path.Combine(globals.UbisoftRootFolder, accountId);
        configManager.Data.DetectedUbiGames.Clear();

        AnsiConsole.MarkupLine("[cyan][[inf]][/] Looking for games..");

        foreach (var gameFolder in Directory.GetDirectories(accountRootFolder)) {
            var gameId = Path.GetFileName(gameFolder);
            var gameName = await utilities.TranslateUbisoftGameId(gameFolder);

            configManager.Data.DetectedUbiGames.Add(gameId);
            AnsiConsole.MarkupLine($"Game found: [yellow]{Markup.Escape(gameName)}[/]");
        }

        if (configManager.Data.DetectedUbiGames.Count == 0) {
            AnsiConsole.MarkupLine("[darkorange3][[warn]][/] No games found!");
        }
        else {
            AnsiConsole.MarkupLine($"[green][[suc]][/] Total games found: {configManager.Data.DetectedUbiGames.Count}");
        }

        configManager.Save();
    }
}