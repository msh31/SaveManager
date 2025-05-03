//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes

using System.Text.Json;
using SaveManager.Managers;
using SaveManager.Models;
using SaveManager.Interfaces;
using Spectre.Console;

class UbiManager : BaseManager, ISaveFileStorage
{
    private readonly ConfigManager _configManager;
    private readonly Utilities _utilities;
    private readonly Globals _globals;
    private readonly Logger _logger;
    private Dictionary<string, List<SaveFileInfo>> Saves { get; set; } = new();
    private string accountRootFolder;
    private List<string> _accountFolders = new();

    public UbiManager(ConfigManager configManager, Utilities utilities, Globals globals, Logger logger) : base(globals, "Ubisoft", logger)
    {
        _configManager = configManager;
        _utilities = utilities;
        _globals = globals;
        _logger = logger;
    }

    #region to-do

    public override async Task BackupSaveGame(string gameId, int saveIndex)
    {
        AnsiConsole.MarkupLine("[red][[err]][/] Backup functionality is not implemented yet.");
    }
    
    public override async Task RestoreSaveGame(string backupId)
    {
        AnsiConsole.MarkupLine("[red][[err]][/] Restore functionality is not fully implemented yet.");
    }
    
    #endregion
    
    #region Features

    public override async Task ListSaveGamesAsync()
    {
        if (Saves.Count == 0 && File.Exists(_globals.UbiSaveInfoFilePath))
        {
            try
            {
                var json = await File.ReadAllTextAsync(_globals.UbiSaveInfoFilePath);
                Saves = JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json);
            }
            catch (Exception ex)
            {
                AnsiConsole.MarkupLine($"[red][[err]][/] Error loading save info: {ex.Message}");
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] No saves found.");
            return;
        }

        AnsiConsole.MarkupLine("[cyan][[inf]][/] Listing all Ubisoft saves:");

        foreach (var entry in Saves)
        {
            string[] parts = entry.Key.Split('_');
            var accountId = parts[0];
            var gameId = parts[1];

            var gameName =
                await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));

            AnsiConsole.Markup($"\n[darkcyan]{Markup.Escape(gameName)} - [/]");
            AnsiConsole.Markup($"[yellow]{gameId} - [/]");
            AnsiConsole.Markup($"[red]Account: {accountId} - [/]");
            AnsiConsole.MarkupLine($"[white]Total Saves: {entry.Value.Count}[/]");

            foreach (var save in entry.Value)
            {
                var sizeInKB = save.FileSize / 1024.0;
                var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                var timestampCreated = save.DateCreated.ToString("yyyy-MM-dd HH:mm:ss");

                var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                AnsiConsole.MarkupLine(
                    $"[gray]   - {Markup.Escape(displayName)} | {sizeInKB:F1}KB | created: {timestampCreated} | modified: {timestamp}[/]");
            }
        }
    }

    public async Task SyncBetweenPlatformsAsync(string id = null)
    {
        if (Saves.Count == 0 && File.Exists(_globals.UbiSaveInfoFilePath))
        {
            try
            {
                var json = await File.ReadAllTextAsync(_globals.UbiSaveInfoFilePath);
                Saves = JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json);
            }
            catch (Exception ex)
            {
                AnsiConsole.MarkupLine($"[red][[err]][/] Error loading save info: {ex.Message}");
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] No saves found.");
            return;
        }
        
        var gameNameToEntries = new Dictionary<string, List<(string Key, List<SaveFileInfo> SaveList)>>();
        
        foreach (var entry in Saves)
        {
            string[] parts = entry.Key.Split('_');
            var accountId = parts[0];
            var gameId = parts[1];
            
            var gameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));
            
            var baseGameName = gameName.Replace(" (Steam)", "").Trim();

            if (!gameNameToEntries.ContainsKey(baseGameName))
            {
                gameNameToEntries[baseGameName] = new List<(string, List<SaveFileInfo>)>();
            }

            gameNameToEntries[baseGameName].Add((entry.Key, entry.Value));
        }
        
        var gamesWithMultiplePlatforms = gameNameToEntries
            .Where(kvp => kvp.Value.Count > 1)
            .ToDictionary(kvp => kvp.Key, kvp => kvp.Value);

        if (gamesWithMultiplePlatforms.Count == 0)
        {
            AnsiConsole.MarkupLine("[yellow][[warn]][/] No games found with multiple platforms for syncing.");
            return;
        }
        
        if (!string.IsNullOrEmpty(id))
        {
            var matchingGame = gamesWithMultiplePlatforms
                .FirstOrDefault(g => g.Value.Any(e => e.Key.Split('_')[1] == id));

            if (matchingGame.Key != null)
            {
                await SyncSpecificGame(matchingGame.Key, matchingGame.Value);
                return;
            }
            else
            {
                AnsiConsole.MarkupLine($"[red][[err]][/] Game with ID {id} not found or doesn't have multiple platforms.");
                return;
            }
        }
        
        AnsiConsole.MarkupLine("\n[cyan][[inf]][/] Games available for syncing (multiple platforms detected):");

        var gameOptions = new List<string>();
        var gameList = gamesWithMultiplePlatforms.ToList();

        for (int i = 0; i < gameList.Count; i++)
        {
            gameOptions.Add(gameList[i].Key);
        }
        
        var selectedGame = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Select a [green]game[/] to sync:")
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more games)[/]")
                .AddChoices(gameOptions));

        var selectedIndex = gameOptions.IndexOf(selectedGame);
        if (selectedIndex >= 0)
        {
            var game = gameList[selectedIndex];
            await SyncSpecificGame(game.Key, game.Value);
        }
    }

    public override async Task RenameSaveFilesAsync(string gameId, string saveFileName, string newDisplayName)
    {
        if (Saves.Count == 0 && File.Exists(_globals.UbiSaveInfoFilePath))
        {
            try
            {
                var json = await File.ReadAllTextAsync(_globals.UbiSaveInfoFilePath);
                Saves = JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json);
            }
            catch (Exception ex)
            {
                AnsiConsole.MarkupLine($"[red][[err]][/] Error loading save info: {ex.Message}");
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] No saves found. Try running 'refresh ubisoft' first.");
            return;
        }
        
        bool foundGame = false;
        bool foundSave = false;
        
        foreach (var entry in Saves)
        {
            string[] parts = entry.Key.Split('_');
            var accountId = parts[0];
            var entryGameId = parts[1];
            
            if (entryGameId == gameId)
            {
                foundGame = true;
                var gameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));
                
                var saveFile = entry.Value.FirstOrDefault(s => s.FileName.Equals(saveFileName, StringComparison.OrdinalIgnoreCase));
                
                if (saveFile != null)
                {
                    foundSave = true;
                    string oldDisplayName = saveFile.DisplayName == "CUSTOM_NAME_NOT_SET" ? saveFile.FileName : saveFile.DisplayName;
                    saveFile.DisplayName = newDisplayName;
                    
                    var json = JsonSerializer.Serialize(Saves, new JsonSerializerOptions { WriteIndented = true });
                    await File.WriteAllTextAsync(_globals.UbiSaveInfoFilePath, json);
                    
                    AnsiConsole.MarkupLine($"[green][[suc]][/] Renamed save in [darkcyan]{Markup.Escape(gameName)}[/]");
                    AnsiConsole.MarkupLine($"[gray]   From: {Markup.Escape(oldDisplayName)}[/]");
                    AnsiConsole.MarkupLine($"[gray]   To: {Markup.Escape(newDisplayName)}[/]");
                    return;
                }
            }
        }
        
        if (!foundGame)
        {
            AnsiConsole.MarkupLine($"[red][[err]][/] Game with ID '{gameId}' not found.");
        }
        else if (!foundSave)
        {
            AnsiConsole.MarkupLine($"[red][[err]][/] Save file '{saveFileName}' not found in game with ID '{gameId}'.");
        }
    }
    
    #endregion

    public override async Task InitializeSaveDetection()
    {
        try
        {
            _accountFolders = GetAccountId();

            if (_accountFolders.Count == 0)
            {
                AnsiConsole.MarkupLine("[red][[err]][/] No Ubisoft accounts found. Unable to proceed.");
                return;
            }

            foreach (var accountFolder in _accountFolders)
            {
                var accountId = Path.GetFileName(accountFolder);
                AnsiConsole.MarkupLine($"[cyan][[inf]][/] Processing account: {accountId}");

                await FindGames(accountId);
                await FindSaveGames();
            }
        }
        catch (Exception ex)
        {
            _logger.Error($"Error initializing save detection: {ex.Message}", 0, true);
            throw;
        }
    }

    #region Helper Methods
    private List<string> GetAccountId()
    {
        AnsiConsole.MarkupLine("[cyan][[inf]][/] Looking for accounts..");
        var accountFolders = new List<string>();

        foreach (var folder in Directory.GetDirectories(_globals.UbisoftRootFolder))
        {
            if (Path.GetFileName(folder).Length > 20)
            {
                accountFolders.Add(folder);
            }
        }

        if (accountFolders.Count == 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] No Ubisoft accounts found!");
            _configManager.Data.DetectedUbiAccount = string.Empty;
            _configManager.Save();
            return accountFolders;
        }

        if (!string.IsNullOrEmpty(_configManager.Data.DetectedUbiAccount))
        {
            if (_configManager.Data.DetectedUbiAccount == "all")
            {
                return accountFolders;
            }

            var existingAccount = Path.Combine(_globals.UbisoftRootFolder, _configManager.Data.DetectedUbiAccount);

            if (Directory.Exists(existingAccount))
            {
                return new List<string> { existingAccount };
            }
        }

        if (accountFolders.Count == 1)
        {
            string accountId = Path.GetFileName(accountFolders[0]);
            AnsiConsole.MarkupLine($"[green][[suc]][/] Found Ubisoft account: {accountId}");
            _configManager.Data.DetectedUbiAccount = accountId;
            _configManager.Save();
            return accountFolders;
        }

        AnsiConsole.MarkupLine($"[cyan][[inf]][/] Found {accountFolders.Count} Ubisoft accounts:");
        var choices = new List<string>();

        foreach (var folder in accountFolders)
        {
            choices.Add(Path.GetFileName(folder));
        }

        choices.Add("All accounts");

        var selectedOption = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Which [yellow]account[/] do you want to use?")
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more accounts)[/]")
                .AddChoices(choices));

        // Process the selection
        if (selectedOption == "All accounts")
        {
            _configManager.Data.DetectedUbiAccount = "all";
            _configManager.Save();
            return accountFolders;
        }

        for (int i = 0; i < accountFolders.Count; i++)
        {
            if (Path.GetFileName(accountFolders[i]) == selectedOption)
            {
                _configManager.Data.DetectedUbiAccount = selectedOption;
                _configManager.Save();
                return new List<string> { accountFolders[i] };
            }
        }

        // This shouldn't happen unless there's a mismatch between the displayed options and accountFolders
        AnsiConsole.MarkupLine("[red][[err]][/] Something went wrong! [error code: 0x0896]");
        return new List<string>();
    }

    private async Task FindGames(string accountId)
    {
        accountRootFolder = Path.Combine(_globals.UbisoftRootFolder, accountId);
        _configManager.Data.DetectedUbiGames.Clear();

        AnsiConsole.MarkupLine("[cyan][[inf]][/] Looking for games..");

        foreach (var gameFolder in Directory.GetDirectories(accountRootFolder))
        {
            var gameId = Path.GetFileName(gameFolder);
            var gameName = await _utilities.TranslateUbisoftGameId(gameFolder);

            _configManager.Data.DetectedUbiGames.Add(gameId);
            AnsiConsole.MarkupLine($"Game found: [yellow]{Markup.Escape(gameName)}[/]");
        }

        if (_configManager.Data.DetectedUbiGames.Count == 0)
        {
            AnsiConsole.MarkupLine("[darkorange3][[warn]][/] No games found!");
        }
        else
        {
            AnsiConsole.MarkupLine(
                $"[green][[suc]][/] Total games found: {_configManager.Data.DetectedUbiGames.Count}");
        }

        _configManager.Save();
    }

    private async Task FindSaveGames(bool ignoreAutoSave = false)
    {
        AnsiConsole.MarkupLine("[cyan][[inf]][/] Looking for savegames..");

        Dictionary<string, Dictionary<string, string>> existingDisplayNames = new Dictionary<string, Dictionary<string, string>>();
        if (File.Exists(_globals.UbiSaveInfoFilePath))
        {
            try
            {
                var json = await File.ReadAllTextAsync(_globals.UbiSaveInfoFilePath);
                var existingSaves = JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json);
            
                if (existingSaves != null)
                {
                    foreach (var saveKey in existingSaves.Keys)
                    {
                        existingDisplayNames[saveKey] = new Dictionary<string, string>();
                        foreach (var save in existingSaves[saveKey])
                        {
                            if (save.DisplayName != "CUSTOM_NAME_NOT_SET")
                            {
                                existingDisplayNames[saveKey][save.FileName] = save.DisplayName;
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                _logger.Warning($"Could not load existing save display names: {ex.Message}", 0, true);
            }
        }
        
        foreach (var gameId in _configManager.Data.DetectedUbiGames)
        {
            var gameFolder = Path.Combine(accountRootFolder, gameId);

            if (!Directory.Exists(gameFolder))
            {
                continue;
            }

            var gameName = await _utilities.TranslateUbisoftGameId(gameFolder);
            var files = Directory.GetFiles(gameFolder);
            var validSaves = new List<SaveFileInfo>();

            foreach (var file in files)
            {
                var fileName = Path.GetFileName(file);

                // ignore these files
                if (fileName.IndexOf("[Options]", StringComparison.OrdinalIgnoreCase) >= 0)
                {
                    continue;
                }

                if (!fileName.EndsWith(".save", StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }

                if (ignoreAutoSave && fileName.IndexOf("[AutSave", StringComparison.OrdinalIgnoreCase) >= 0)
                {
                    continue;
                }

                var fileInfo = new FileInfo(file);

                var saveInfo = new SaveFileInfo
                {
                    FileName = fileName,
                    FileSize = fileInfo.Length,
                    LastModified = fileInfo.LastWriteTime,
                    DateCreated = fileInfo.CreationTime,
                    DisplayName = "CUSTOM_NAME_NOT_SET"
                };

                var saveDisplayNameKey = $"{Path.GetFileName(accountRootFolder)}_{gameId}";
                if (existingDisplayNames.ContainsKey(saveDisplayNameKey) && existingDisplayNames[saveDisplayNameKey].ContainsKey(fileName))
                {
                    saveInfo.DisplayName = existingDisplayNames[saveDisplayNameKey][fileName];
                }

                validSaves.Add(saveInfo);
            }

            if (!validSaves.Any())
            {
                continue;
            }

            var saveKey = $"{Path.GetFileName(accountRootFolder)}_{gameId}";
            Saves[saveKey] = validSaves;
            var json = JsonSerializer.Serialize(Saves, new JsonSerializerOptions { WriteIndented = true });
            await File.WriteAllTextAsync(_globals.UbiSaveInfoFilePath, json);

            AnsiConsole.MarkupLine($"[red]{Markup.Escape(gameName)} - {validSaves.Count} Detected Saves[/]");

            foreach (var save in validSaves)
            {
                double sizeInKB = save.FileSize / 1024.0;
                string timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                string timestampCreated = save.DateCreated.ToString("yyyy-MM-dd HH:mm:ss");

                var escapedFileName = Markup.Escape(save.FileName);
                
                AnsiConsole.MarkupLine($"[darkcyan]   - {escapedFileName} | {sizeInKB:F1}KB | created: {timestampCreated} | updated: {timestamp}[/]");
            }
        }
    }

    private async Task SyncSpecificGame(string gameName, List<(string Key, List<SaveFileInfo> SaveList)> platforms)
    {
        AnsiConsole.MarkupLine($"\n[cyan][[inf]][/] Syncing {gameName}");

        var table = new Table()
            .Title($"[bold]{gameName}[/] Platforms")
            .Border(TableBorder.Rounded)
            .AddColumn(new TableColumn("Platform").Centered())
            .AddColumn(new TableColumn("Saves").Centered())
            .AddColumn(new TableColumn("Account ID").Centered())
            .AddColumn(new TableColumn("Game ID").Centered());

        for (int i = 0; i < platforms.Count; i++)
        {
            string[] parts = platforms[i].Key.Split('_');
            var accountId = parts[0];
            var gameId = parts[1];
            var fullGameName =
                await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));
            var saveCount = platforms[i].SaveList.Count;

            table.AddRow(
                $"[cyan]{fullGameName}[/]",
                $"{saveCount}",
                $"[gray]{accountId}[/]",
                $"[yellow]{gameId}[/]"
            );
        }

        AnsiConsole.Write(table);
        
// source
        var platformOptions = new string[platforms.Count];
        for (int i = 0; i < platforms.Count; i++)
        {
            string[] parts = platforms[i].Key.Split('_');
            var gameId = parts[1];
            var fullGameName =
                await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, parts[0], gameId));
            platformOptions[i] = fullGameName;
        }

        var sourcePlatformName = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Select [green]source[/] platform:")
                .PageSize(10)
                .AddChoices(platformOptions));

        int sourcePlatformIndex = Array.IndexOf(platformOptions, sourcePlatformName);
        
        var saveOptions = new List<string>();
        for (int i = 0; i < platforms[sourcePlatformIndex].SaveList.Count; i++)
        {
            var save = platforms[sourcePlatformIndex].SaveList[i];
            var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
            saveOptions.Add(displayName);
        }

        if (saveOptions.Count == 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] No save files found for this platform.");
            return;
        }

        var selectedSaveName = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Select [green]save file[/] to sync:")
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more saves)[/]")
                .AddChoices(saveOptions));

        int sourceFileIndex = saveOptions.IndexOf(selectedSaveName);

// destination
        var destPlatformOptions = new List<string>();
        for (int i = 0; i < platforms.Count; i++)
        {
            if (i != sourcePlatformIndex)
            {
                string[] parts = platforms[i].Key.Split('_');
                var gameId = parts[1];
                var fullGameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, parts[0], gameId));
                destPlatformOptions.Add(fullGameName);
            }
        }

        if (destPlatformOptions.Count == 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] No other platforms available for syncing.");
            return;
        }

        var destPlatformName = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Select [green]destination[/] platform:")
                .PageSize(10)
                .AddChoices(destPlatformOptions));
        
        int adjustedIndex = destPlatformOptions.IndexOf(destPlatformName);
        int destPlatformIndex = -1;

        for (int i = 0, adjusted = 0; i < platforms.Count; i++)
        {
            if (i != sourcePlatformIndex)
            {
                if (adjusted == adjustedIndex)
                {
                    destPlatformIndex = i;
                    break;
                }

                adjusted++;
            }
        }

        if (destPlatformIndex < 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] Error identifying destination platform.");
            return;
        }
        
        var sourceSave = platforms[sourcePlatformIndex].SaveList[sourceFileIndex];
        var sourceParts = platforms[sourcePlatformIndex].Key.Split('_');
        var sourceAccountId = sourceParts[0];
        var sourceGameId = sourceParts[1];
        var sourceFilePath = Path.Combine(_globals.UbisoftRootFolder, sourceAccountId, sourceGameId, sourceSave.FileName);
        
        var destParts = platforms[destPlatformIndex].Key.Split('_');
        var destAccountId = destParts[0];
        var destGameId = destParts[1];
        var destDirectory = Path.Combine(_globals.UbisoftRootFolder, destAccountId, destGameId);
        var destFilePath = Path.Combine(destDirectory, sourceSave.FileName);
        
        bool overwrite = false;
        if (File.Exists(destFilePath))
        {
            overwrite = AnsiConsole.Confirm($"[yellow][[warn]][/] File {sourceSave.FileName} already exists in destination. Overwrite?", false);

            if (!overwrite)
            {
                AnsiConsole.MarkupLine("[cyan][[inf]][/] Operation cancelled.");
                return;
            }
            
            var backupDir = Path.Combine(Globals.BackupsFolder, "SyncBackups", destGameId);
            Directory.CreateDirectory(backupDir);
            var backupPath = Path.Combine(backupDir, $"{sourceSave.FileName}_{DateTime.Now:yyyyMMdd_HHmmss}");

            try
            {
                File.Copy(destFilePath, backupPath);
                AnsiConsole.MarkupLine($"[cyan][[inf]][/] Created backup of destination file at: {backupPath}");
            }
            catch (Exception ex)
            {
                AnsiConsole.MarkupLine($"[yellow][[warn]][/] Could not create backup: {ex.Message}");
                var proceedAnyway = AnsiConsole.Confirm("Proceed anyway?", false);
                if (!proceedAnyway)
                {
                    AnsiConsole.MarkupLine("[cyan][[inf]][/] Operation cancelled.");
                    return;
                }
            }
        }
        
        await AnsiConsole.Status().StartAsync("Copying save file...", async ctx =>
            {
                ctx.Spinner(Spinner.Known.Dots);

                try
                {
                    File.Copy(sourceFilePath, destFilePath, overwrite);
                    
                    var destSaveIndex = platforms[destPlatformIndex].SaveList
                        .FindIndex(s => s.FileName == sourceSave.FileName);
                    if (destSaveIndex >= 0)
                    {
                        platforms[destPlatformIndex].SaveList[destSaveIndex].LastModified = DateTime.Now;
                    }
                    else
                    {
                        var newSaveInfo = new SaveFileInfo
                        {
                            FileName = sourceSave.FileName,
                            FileSize = new FileInfo(destFilePath).Length,
                            LastModified = DateTime.Now,
                            DateCreated = DateTime.Now,
                            DisplayName = sourceSave.DisplayName
                        };

                        platforms[destPlatformIndex].SaveList.Add(newSaveInfo);
                    }
                    
                    var json = JsonSerializer.Serialize(Saves, new JsonSerializerOptions { WriteIndented = true });
                    await File.WriteAllTextAsync(_globals.UbiSaveInfoFilePath, json);

                    ctx.Status("Save file copied successfully!");
                    await Task.Delay(500);
                }
                catch (Exception ex)
                {
                    AnsiConsole.MarkupLine($"[red][[err]][/] Error copying file: {ex.Message}");
                }
            });

        AnsiConsole.MarkupLine($"[green][[suc]][/] Successfully synced save from {sourceGameId} to {destGameId}!");
    }
#endregion
}  