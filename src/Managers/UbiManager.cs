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

    public UbiManager (ConfigManager configManager, Utilities utilities, Globals globals, Logger logger) : base(globals, "Ubisoft", logger)
    {
        _configManager = configManager;
        _utilities = utilities;
        _globals = globals;
        _logger = logger;
    }

    public async Task<string> GetSavePath(string gameId, string fileName)
    {
        return Path.Combine(Globals.UbisoftRootFolder, _configManager.Data.DetectedUbiAccount, gameId, fileName);
    }
    
    public async Task<bool> ExportSave(string gameId, string fileName, string destinationPath)
    {
        // TODO
        return true;
    }
    
    public async Task<bool> ImportSave(string gameId, string filePath)
    {
        // TODO
        return true;
    }
    
    public override async Task RestoreSaveGame(string backupId)
    {
        // TODO
        AnsiConsole.Write(new Markup("[cyan][[inf]][/] Restore functionality not fully implemented yet.\n"));
    }
    
    public override async Task BackupSaveGame(string gameId, int saveIndex)
    {
        if (Saves.Count == 0 && File.Exists(Globals.UbiSaveInfoFilePath))
        {
            try
            {
                var json = await File.ReadAllTextAsync(Globals.UbiSaveInfoFilePath);
                Saves = JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json);
            }
            catch (Exception ex)
            {
                AnsiConsole.Write(new Markup($"[red][[err]][/] Error loading save info: {ex.Message}\n"));
                return;
            }
        }
        
        if (Saves == null || Saves.Count == 0)
        {
            AnsiConsole.Write(new Markup("[red][[err]][/] No saves found to backup.\n"));
            return;
        }
        
        var saveKey = $"{_configManager.Data.DetectedUbiAccount}_{gameId}";
        
        if (!Saves.ContainsKey(saveKey))
        {
            AnsiConsole.Write(new Markup($"[red][[err]][/] No saves found for game: {gameId}\n"));
            return;
        }
        
        if (saveIndex < 0 || saveIndex >= Saves[saveKey].Count)
        {
            AnsiConsole.Write(new Markup($"[red][[err]][/] Invalid save index: {saveIndex}\n"));
            return;
        }
        
        var saveFile = Saves[saveKey][saveIndex];
        var savePath = Path.Combine(Globals.UbisoftRootFolder, _configManager.Data.DetectedUbiAccount, gameId, saveFile.FileName);
        var saveName = saveFile.DisplayName == "CUSTOM_NAME_NOT_SET" ? saveFile.FileName : saveFile.DisplayName;
        
        var backupPath = await CreateBackup(savePath, gameId, saveName);
        
        if (backupPath != null)
        {
            AnsiConsole.Write(new Markup($"[green][[suc]][/] Successfully backed up: {saveName}\n"));
        }
    }
    
    public Task<List<SaveFileInfo>> GetSaveFiles(string gameId) 
    {
        // TODO
        return null; //dont test this feature yet
    }
    
    public Task<bool> SaveDisplayName(string gameId, string fileName, string displayName)
    {
        // TODO
        return null; //dont test this feature yet
    }
    
    public async Task SyncBetweenPlatformsAsync(string id)
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
                AnsiConsole.Write(new Markup($"[red][[err]][/] Error loading save info: {ex.Message} \n"));
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            AnsiConsole.Write(new Markup($"[red][[err]][/] No saves found.\n"));
            return;
        }
        
        try
        {
            AnsiConsole.Write(new Markup($"\n[cyan][[inf]][/] Processing {Saves.Count} game entries..."));
            
            var groupedByName = Saves
                .GroupBy(kvp => kvp.Value) 
                .Where(group => group.Count() > 1); 
            
            var duplicateGameIds = groupedByName
                .ToDictionary(group => group.Key,  
                              group => group.Select(kvp => kvp.Key).ToList()); 


            if (duplicateGameIds.Any())
            {
                Console.WriteLine("\nGames with the same name found and their corresponding IDs:");
                foreach (var kvp in duplicateGameIds)
                {
                    Console.WriteLine($"  Game Name: '{kvp.Key}'");
                    var sortedIds = kvp.Value.Select(int.Parse).OrderBy(id => id).Select(id => id.ToString());
                    Console.WriteLine($"    IDs      : {string.Join(", ", sortedIds)}");
                }
            }
            else
            {
                Console.WriteLine("\nNo games found with duplicate names in the list.");
            }
        }
        catch (JsonException e)
        {
            Console.WriteLine($"\nError parsing JSON data: {e.Message}");
            Console.WriteLine("The content fetched from the URL might not be valid JSON.");
        }
        catch (Exception e)
        {
            Console.WriteLine($"\nAn unexpected error occurred: {e.Message}");
        }
    }
    
    public override async Task RenameSaveFilesAsync(string id)
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
                AnsiConsole.Write(new Markup($"[red][[err]][/] Error loading save info: {ex.Message}\n"));
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            AnsiConsole.Write(new Markup($"[red][[err]][/] No saves found.\n"));
            return;
        }
        
        bool gameFound = false;
        foreach (var entry in Saves)
        {
            string[] parts = entry.Key.Split('_');
            var gameId = parts[1];

            if (gameId.Equals(id, StringComparison.OrdinalIgnoreCase))
            {
                gameFound = true;
                var accountId = parts[0];
                var gameName = _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId)).Result;

                AnsiConsole.Write(new Markup($"[green][[suc]][/] All Savegames for: {gameName}\n\n"));
                
                int index = 0;
                foreach (var save in entry.Value)
                {
                    var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                    AnsiConsole.Write(new Markup($"[[{index}]] {displayName}\n"));
                    index++;
                }
                
                Console.WriteLine(string.Empty);
                AnsiConsole.Write(new Markup("[cyan][[inf]][/] Enter the index of the save file to rename ('cancel' or 'c' to abort):"));
                var input = Console.ReadLine();

                if (input.ToLower() is "cancel" or "c")
                {
                    AnsiConsole.Write(new Markup("[cyan][[inf]][/] Operation cancelled.\n"));
                    return;
                }

                if (int.TryParse(input, out int saveIndex) && saveIndex >= 0 && saveIndex < entry.Value.Count)
                {
                    AnsiConsole.Write(new Markup($"[cyan][[inf]][/] Current name: {entry.Value[saveIndex].DisplayName}\n"));
                    AnsiConsole.Write(new Markup("[cyan][[inf]][/] Enter new display name: "));
                    var newName = Console.ReadLine();

                    if (!string.IsNullOrWhiteSpace(newName))
                    {
                        entry.Value[saveIndex].DisplayName = newName;
                        
                        var json = JsonSerializer.Serialize(Saves, new JsonSerializerOptions { WriteIndented = true });
                        File.WriteAllText(_globals.UbiSaveInfoFilePath, json);

                        AnsiConsole.Write(new Markup($"[green][[suc]][/] Save file renamed to: {newName}\n"));
                    }
                    else
                    {
                        AnsiConsole.Write(new Markup($"[red][[err]][/] Invalid name. Operation cancelled.\n"));
                    }
                }
                else
                {
                    AnsiConsole.Write(new Markup($"[red][[err]][/] Invalid selection. Operation cancelled.\n"));
                }
            }
        }

        if (!gameFound)
        {
            AnsiConsole.Write(new Markup($"[red][[err]][/] The ID: {id} was not found in the detected games list.\n"));
        }
    }

    public override async Task InitializeSaveDetection()
    {
        try
        {
            _accountFolders = GetAccountId();

            if (_accountFolders.Count == 0)
            {
                AnsiConsole.Write(new Markup("[red][[err]][/] No Ubisoft accounts found. Unable to proceed.\n"));
                return;
            }

            foreach (var accountFolder in _accountFolders)
            {
                var accountId = Path.GetFileName(accountFolder);
                AnsiConsole.Write(new Markup($"[cyan][[inf]][/] Processing account: {accountId}\n"));

                await FindGames(accountId);
                await FindSaveGames();
            }
        }
        catch (Exception ex)
        {
            _logger.Error($"[red][[err]][/] Error initializing save detection\n {ex.Message}", 0, true);
            throw;
        }

        _accountFolders = GetAccountId();

        if (_accountFolders.Count == 0)
        {
            _logger.Error("[red][[err]][/] No Ubisoft accounts found. Unable to proceed with game detection.", 0, true);
            return;
        }

        foreach (var accountFolder in _accountFolders)
        {
            string accountId = Path.GetFileName(accountFolder);
            AnsiConsole.Write(new Markup($"[cyan][[inf]][/] Processing account: {accountId}\n"));

            await FindGames(accountId);
            await FindSaveGames();
        }
    }
    
// helper methods    
    private List<string> GetAccountId()
    {
        AnsiConsole.Write(new Markup("[cyan][[inf]][/] Looking for accounts..\n"));
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
            AnsiConsole.Write(new Markup("[red][[err]][/] No Ubisoft accounts found!\n"));
            _configManager.Data.DetectedUbiAccount = string.Empty;
            _configManager.Save();
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
            return accountFolders;
        }

        if (accountFolders.Count == 1)
        {
            string accountId = Path.GetFileName(accountFolders[0]);
            AnsiConsole.Write(new Markup($"[green][[suc]][/] Found Ubisoft account: {accountId}\n"));
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
        
        var selectedOptions = AnsiConsole.Prompt(
            new SelectionPrompt<string>()  // Using SelectionPrompt instead of MultiSelectionPrompt
                .Title("Which [yellow]account[/] do you want to use?")
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more accounts)[/]")
                .AddChoices(choices));

// Process the selection
        if (selectedOptions == "All accounts")
        {
            _configManager.Data.DetectedUbiAccount = "all";
            _configManager.Save();
            return accountFolders;
        }
        
        for (int i = 0; i < accountFolders.Count; i++)
        {
            if (Path.GetFileName(accountFolders[i]) == selectedOptions)
            {
                _configManager.Data.DetectedUbiAccount = selectedOptions;
                _configManager.Save();
                return new List<string> { accountFolders[i] };
            }
        }
    
        // This shouldn't happen unless there's a mismatch between the displayed options and accountFolders
        AnsiConsole.MarkupLine($"[red][[err]][/] something went wrong! [error code: 0x0896]");
        return new List<string>();
    }

    private async Task FindGames(string accountId)
    {
        accountRootFolder = Path.Combine(_globals.UbisoftRootFolder, accountId);
        _configManager.Data.DetectedUbiGames.Clear();

        AnsiConsole.Write(new Markup($"[cyan][[inf]][/] Looking for games..\n"));

        foreach (var gameFolder in Directory.GetDirectories(accountRootFolder))
        {
            var gameId = Path.GetFileName(gameFolder);
            var gameName = await _utilities.TranslateUbisoftGameId(gameFolder);

            _configManager.Data.DetectedUbiGames.Add(gameId);
            AnsiConsole.Write(new Markup($"Game found: [yellow]{gameName}[/] \n"));
        }

        if (_configManager.Data.DetectedUbiGames.Count == 0)
        {
            AnsiConsole.Write(new Markup($"[red][[err]][/] No games found!\n"));
        }
        else
        {
            AnsiConsole.Write(new Markup($"[green][[suc]][/] Total games found: {{_configManager.Data.DetectedUbiGames.Count\n"));
        }

        _configManager.Save();
    }

    private async Task FindSaveGames(bool ignoreAutoSave = false)
    {
        AnsiConsole.Write(new Markup($"[cyan][[inf]][/] Looking for savegames..\n"));

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
                    FileSize = fileInfo.Length / 1024,
                    LastModified = fileInfo.LastWriteTime,
                    DateCreated = fileInfo.CreationTime,
                    DisplayName = "CUSTOM_NAME_NOT_SET"
                };

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

            AnsiConsole.Write(new Markup($"[red][[err]] {gameName} - {validSaves.Count} Detected Saves[/] \n"));

            foreach (var save in validSaves)
            {
                double sizeInKB = save.FileSize / 1024.0;
                string timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                string timestampCreated = save.DateCreated.ToString("yyyy-MM-dd HH:mm:ss");
                AnsiConsole.Write(new Markup($"[darkcyan]   - {save.FileName} | {sizeInKB:F1}KB | created: {timestampCreated} | updated: {timestamp}[/] \n"));
            }
        }
    }

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
                AnsiConsole.Write(new Markup($"[red][[err]][/] Error loading save info: {ex.Message}\n"));
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            AnsiConsole.Write(new Markup($"[red][[err]][/] No saves found.\n"));
            return;
        }

        AnsiConsole.Write(new Markup($"[cyan][[inf]][/] Listing all Ubisoft saves:\n"));

        foreach (var entry in Saves)
        {
            string[] parts = entry.Key.Split('_');
            var accountId = parts[0];
            var gameId = parts[1];

            var gameName = _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId)).Result;

            AnsiConsole.Write(new Markup($"\n[darkcyan]{gameName} - [/]"));
            AnsiConsole.Write(new Markup($"[yellow]{gameId} - [/]"));
            AnsiConsole.Write(new Markup($"[red]Account: {accountId} - [/]"));
            AnsiConsole.Write(new Markup($"[white]Total Saves: {entry.Value.Count}[/]\n"));

            foreach (var save in entry.Value)
            {
                var sizeInKB = save.FileSize / 1024.0;
                var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                var timestampCreated = save.DateCreated.ToString("yyyy-MM-dd HH:mm:ss");

                var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                AnsiConsole.Write(new Markup($"[gray]   - {displayName} | {sizeInKB:F1}KB | created: {timestampCreated} | modified: {timestamp}[/] \n"));
            }
        }
    }
}