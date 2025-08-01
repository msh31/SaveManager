using System.Text.Json;
using SaveManager.Managers;
using SaveManager.Models;
using Spectre.Console;
namespace SaveManager.Services;

public class SaveManager(Globals globals, Utilities.Utilities utilities, ConfigManager configManager, Logger logger)
{

    public async Task InitializeAsync()
    {
        var accountFolders = utilities.GetAccountId(globals, configManager);
        
        if (accountFolders.Count == 0) {
            AnsiConsole.MarkupLine(Globals.NoAccountsFoundMessage);
            return;
        }

        foreach (var accountFolder in accountFolders) {
            var accountId = Path.GetFileName(accountFolder) ?? string.Empty;
            AnsiConsole.MarkupLine(string.Format(Globals.ProcessingAccountMessage, accountId));
            
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
        AnsiConsole.MarkupLine(Globals.LookingForSavesMessage);
        
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
        var fileName = Path.GetFileName(filePath) ?? string.Empty;

        if (fileName.Contains(Globals.OptionsFilePattern, StringComparison.OrdinalIgnoreCase)) {
            return false;
        }

        if (!fileName.EndsWith(Globals.SaveFileExtension, StringComparison.OrdinalIgnoreCase)) {
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
            var allSaves = JsonSerializer.Deserialize(json, JsonContext.Default.DictionaryStringListSaveFileInfo);
            
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
                    var sizeInKb = save.FileSizeBytes / Globals.BytesToKb;
                    var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                    var displayName = save.DisplayName == Globals.DefaultDisplayName ? save.FileName : save.DisplayName;
                    
                    AnsiConsole.MarkupLine($"[gray]   - {Markup.Escape(displayName)} | {sizeInKb:F1}KB | modified: {timestamp}[/]");
                }
            }
        } catch (Exception ex) {
            AnsiConsole.MarkupLine($"[red][[err]][/] Error loading {platform} save info: {ex.Message}");
        }
    }

    public async Task PrepareBackup()
    {
        try
        {
            var allSaves = await LoadAllSavesData();
            if (allSaves == null) {
                return;
            }

            var (individualChoices, gameChoices) = await BuildSaveChoices(allSaves);
            if (individualChoices.Count == 0) {
                AnsiConsole.MarkupLine("[red][[err]][/] No saves available for backup.");
                return;
            }

            var selectedSaves = PromptForSaveSelection(individualChoices, gameChoices);
            if (selectedSaves == null) {
                AnsiConsole.MarkupLine("[yellow][[warn]][/] Backup cancelled.");
                return;
            }

            var finalSaves = ProcessSaveSelections(selectedSaves, individualChoices);
            await SaveBackupSelections(finalSaves);
            await AddSavesToBackedUpGames(finalSaves, allSaves);

            AnsiConsole.MarkupLine($"[green][[suc]][/] Selected {finalSaves.Count} saves for backup:");
            foreach (string save in finalSaves) {
                AnsiConsole.WriteLine($"  - {save}");
            }

            await CreateBackup();
        } catch (Exception ex) {
            AnsiConsole.MarkupLine($"[red][[err]][/] Error loading saves: {ex.Message}");
        }
    }

    private async Task CreateBackup()
    {
        var backupFolder = Path.Combine(globals.BackupsFolder, $"{DateTime.Now.ToString("yyyy-MM-dd HH-mm-ss")}");
        var json = await File.ReadAllTextAsync(globals.UbiSaveInfoFilePath);
        var allSaves = JsonSerializer.Deserialize(json, JsonContext.Default.DictionaryStringListSaveFileInfo);
        
        Directory.CreateDirectory(backupFolder);
        var accountBackupFolder = Path.Combine(backupFolder, configManager.Data.DetectedUbiAccount);
        Directory.CreateDirectory(accountBackupFolder);

        foreach (var gameId in configManager.Data.BackedUpGames) {
            var saveKey = $"{configManager.Data.DetectedUbiAccount}_{gameId}";

            if (!allSaves.ContainsKey(saveKey)) {
                continue;
            }
            
            var saves = allSaves[saveKey];
            var gameBackupFolder = Path.Combine(accountBackupFolder, gameId);
                
            AnsiConsole.MarkupLine($"[cyan][[inf]][/] Creating backup for {gameId}");
            Directory.CreateDirectory(gameBackupFolder);
                
            foreach (var save in saves) {
                var destinationPath = Path.Combine(gameBackupFolder, save.FileName);
                File.Copy(save.FilePath, destinationPath, overwrite: true);
                
                AnsiConsole.MarkupLine($"[green]Copied:[/] {save.FileName}");
            }
        }
    }

    private async Task<Dictionary<string, List<SaveFileInfo>>?> LoadAllSavesData()
    {
        var dataFilePath = Path.Combine(Path.GetDirectoryName(globals.ConfigFilePath) ?? "", globals.UbiSaveInfoFilePath);
        
        if (!File.Exists(dataFilePath)) {
            AnsiConsole.MarkupLine("[red][[err]][/] No saves found.");
            return null;
        }

        var json = await File.ReadAllTextAsync(dataFilePath);
        var allSaves = JsonSerializer.Deserialize(json, JsonContext.Default.DictionaryStringListSaveFileInfo);
        
        if (allSaves == null || allSaves.Count == 0) {
            AnsiConsole.MarkupLine("[red][[err]][/] No saves found.");
            return null;
        }

        return allSaves;
    }

    private async Task<(List<string> individualChoices, List<string> gameChoices)> BuildSaveChoices(Dictionary<string, List<SaveFileInfo>> allSaves)
    {
        var individualChoices = new List<string>();
        var gameChoices = new List<string>();

        foreach (var saveGroup in allSaves) {
            var parts = saveGroup.Key.Split('_', 2);
            var accountId = parts[0];
            var gameId = parts[1];
            
            var gameName = await utilities.TranslateUbisoftGameId(Path.Combine(globals.UbisoftRootFolder, accountId, gameId));
            gameChoices.Add($"[[All from {Markup.Escape(gameName)}]]");
            
            foreach (var save in saveGroup.Value) {
                var displayName = save.DisplayName == Globals.DefaultDisplayName ? save.FileName : save.DisplayName;
                var sizeInKb = save.FileSizeBytes / Globals.BytesToKb;
                var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm");
                
                individualChoices.Add($"{Markup.Escape(gameName)} | {Markup.Escape(displayName)} | {sizeInKb:F1}KB | {timestamp}");
            }
        }

        return (individualChoices, gameChoices);
    }

    private List<string>? PromptForSaveSelection(List<string> individualChoices, List<string> gameChoices)
    {
        var allChoices = new List<string> { "[[All saves]]", "[[Cancel]]" };
        
        if (gameChoices.Count > 0) {
            allChoices.Add("─── Game Groups ───");
            allChoices.AddRange(gameChoices);
        }
        
        if (individualChoices.Count > 0) {
            allChoices.Add("─── Individual Saves ───");
            allChoices.AddRange(individualChoices);
        }
        var selectedSaves = AnsiConsole.Prompt(
            new MultiSelectionPrompt<string>()
                .Title("Select saves to [green]backup[/]:")
                .NotRequired()
                .PageSize(20)
                .MoreChoicesText("[grey](Move up and down to reveal more saves)[/]")
                .InstructionsText(
                    "[grey](Press [blue]<space>[/] to toggle, " + 
                    "[green]<enter>[/] to accept)[/]")
                .AddChoices(allChoices.Where(c => !c.StartsWith("───")).ToArray()));

        if (selectedSaves.Count == 0 || selectedSaves.Contains("[[Cancel]]")) {
            return null;
        }

        return selectedSaves;
    }

    private List<string> ProcessSaveSelections(List<string> selectedSaves, List<string> individualChoices)
    {
        var finalSaves = new List<string>();

        if (selectedSaves.Contains("[[All saves]]")) {
            finalSaves.AddRange(individualChoices);
        } else {
            foreach (var selection in selectedSaves) {
                if (selection == "[[Cancel]]") {
                } else if (selection.StartsWith("[[All from ") && selection.EndsWith("]]")) {
                    var gameNameInBrackets = selection.Substring(11, selection.Length - 13);
                    var matchingGameSaves = individualChoices.Where(c => c.StartsWith(gameNameInBrackets + " |")).ToList();
                    finalSaves.AddRange(matchingGameSaves);
                } else if (!selection.StartsWith("[[")) {
                    finalSaves.Add(selection);
                }
            }
        }

        return finalSaves.Distinct().ToList();
    }

    private async Task AddSavesToBackedUpGames(List<string> finalSaves, Dictionary<string, List<SaveFileInfo>> allSaves)
    {
        var gameIds = new HashSet<string>();

        foreach (var save in finalSaves) {
            foreach (var saveGroup in allSaves) {
                var parts = saveGroup.Key.Split('_', 2);
                
                if (parts.Length != 2) {
                    continue;
                }
                
                var gameId = parts[1];
                var gameName = await utilities.TranslateUbisoftGameId(Path.Combine(globals.UbisoftRootFolder, parts[0], gameId));

                if (!save.StartsWith(gameName + " |")) {
                    continue;
                }
                    
                gameIds.Add(gameId);
                break;
            }
        }

        foreach (var gameId in gameIds) {
            if (!configManager.Data.BackedUpGames.Contains(gameId)) {
                configManager.Data.BackedUpGames.Add(gameId);
            }
        }

        configManager.Save();
        
        if (gameIds.Count > 0) {
            AnsiConsole.MarkupLine($"[cyan][[inf]][/] Added {gameIds.Count} games to backed up games list.");
        }
    }

    private async Task SaveBackupSelections(List<string> selections)
    {
        try {
            var selectionsData = new BackupSelections {
                LastUpdated = DateTime.Now,
                Selections = selections
            };
            
            var json = JsonSerializer.Serialize(selectionsData, JsonContext.Default.BackupSelections);
            await File.WriteAllTextAsync(globals.BackupSelectionsFilePath, json);
        } catch (Exception ex) {
            logger.Error($"Error saving backup selections: {ex.Message}");
        }
    }
    public async Task ListBackupSelections()
    {
        if (!File.Exists(globals.BackupSelectionsFilePath)) {
            AnsiConsole.MarkupLine("[yellow][[warn]][/] No backup selections found. Run 'backup' first to select saves.");
            return;
        }

        try {
            var json = await File.ReadAllTextAsync(globals.BackupSelectionsFilePath);
            var data = JsonSerializer.Deserialize<BackupSelections>(json, JsonContext.Default.BackupSelections);
            
            if (data == null || data.Selections == null) {
                AnsiConsole.MarkupLine("[red][[err]][/] Invalid backup selections data.");
                return;
            }

            AnsiConsole.MarkupLine($"[cyan][[inf]][/] Current backup selections ({data.Selections.Count} saves):");
            AnsiConsole.MarkupLine($"[gray]Last updated: {data.LastUpdated:yyyy-MM-dd HH:mm:ss}[/]\n");

            foreach (var selection in data.Selections) {
                AnsiConsole.WriteLine($"  - {selection}");
            }        } catch (Exception ex) {
            AnsiConsole.MarkupLine($"[red][[err]][/] Error loading backup selections: {ex.Message}");
        }
    }
    
    public async Task RenameSave(string gameId, string fileName, string newName)    {        var savesData = await LoadExistingDisplayNames();
        bool updated = false;

        foreach (var accountEntry in savesData)
        {
            if (accountEntry.Value.ContainsKey(gameId))
            {
                var gameData = accountEntry.Value[gameId];
                if (gameData.ContainsKey(fileName))
                {
                    var gameName = await utilities.TranslateUbisoftGameId(Path.Combine(globals.UbisoftRootFolder, accountEntry.Key, gameId));
                    string oldDisplayName = gameData[fileName] == Globals.DefaultDisplayName ? fileName : gameData[fileName];
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
            var allSaves = JsonSerializer.Deserialize(json, JsonContext.Default.DictionaryStringListSaveFileInfo);
            
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
            var displayName = save.DisplayName == Globals.DefaultDisplayName ? save.FileName : save.DisplayName;
            var escapedDisplayName = Markup.Escape(displayName);
            
            AnsiConsole.MarkupLine($"[darkcyan]   - {escapedDisplayName} | {sizeInKb:F1}KB | created: {timestamp} | updated: {timestamp}[/]");
        }
    }

    private async Task SaveToFile(Dictionary<string, List<SaveFileInfo>> allSaves)
    {
        try {
            var dataFile = Path.Combine(Path.GetDirectoryName(globals.ConfigFilePath) ?? "", globals.UbiSaveInfoFilePath);
            var json = JsonSerializer.Serialize(allSaves, JsonContext.Default.DictionaryStringListSaveFileInfo);
            await File.WriteAllTextAsync(dataFile, json);
        } catch (Exception ex) {
            logger.Error($"Error saving to file: {ex.Message}");
            AnsiConsole.MarkupLine($"[red][[err]][/] Error saving data: {ex.Message}");
        }
    }

    private SaveFileInfo CreateSaveInfo(string filePath, string accountId, string gameId, Dictionary<string, Dictionary<string, Dictionary<string, string>>> existingNames)
    {
        var fileName = Path.GetFileName(filePath) ?? string.Empty;
        var fileInfo = new FileInfo(filePath);
        
        var displayName = Globals.DefaultDisplayName;
        
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

            var json = JsonSerializer.Serialize(allSaves, JsonContext.Default.DictionaryStringListSaveFileInfo);
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
            var gameId = Path.GetFileName(gameFolder) ?? string.Empty;
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