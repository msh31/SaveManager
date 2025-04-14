//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes

using System.Text.Json;
using SaveManager.Managers;
using SaveManager.Models;
using SaveManager.Interfaces;

class UbiManager : BaseManager, ISaveFileStorage
{
    private readonly TerminalUI _terminalUI;
    private readonly ConfigManager _configManager;
    private readonly Utilities _utilities;
    private readonly Globals _globals;
    private Dictionary<string, List<SaveFileInfo>> Saves { get; set; } = new();
    private string accountRootFolder;
    private List<string> _accountFolders = new();

    public UbiManager(TerminalUI terminalUI, ConfigManager configManager, Utilities utilities, Globals globals) : base(terminalUI, globals, "Ubisoft")
    {
        _terminalUI = terminalUI;
        _configManager = configManager;
        _utilities = utilities;
        _globals = globals;
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
        TerminalUI.WriteFormattedTextByType("Restore functionality not fully implemented yet.", "inf", true, false);
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
                TerminalUI.WriteFormattedTextByType($"Error loading save info: {ex.Message}", "err", true, false);
                return;
            }
        }
        
        if (Saves == null || Saves.Count == 0)
        {
            TerminalUI.WriteFormattedTextByType("No saves found to backup.", "err", true, false);
            return;
        }
        
        var saveKey = $"{_configManager.Data.DetectedUbiAccount}_{gameId}";
        
        if (!Saves.ContainsKey(saveKey))
        {
            TerminalUI.WriteFormattedTextByType($"No saves found for game: {gameId}", "err", true, false);
            return;
        }
        
        if (saveIndex < 0 || saveIndex >= Saves[saveKey].Count)
        {
            TerminalUI.WriteFormattedTextByType($"Invalid save index: {saveIndex}", "err", true, false);
            return;
        }
        
        var saveFile = Saves[saveKey][saveIndex];
        var savePath = Path.Combine(Globals.UbisoftRootFolder, _configManager.Data.DetectedUbiAccount, gameId, saveFile.FileName);
        var saveName = saveFile.DisplayName == "CUSTOM_NAME_NOT_SET" ? saveFile.FileName : saveFile.DisplayName;
        
        var backupPath = await CreateBackup(savePath, gameId, saveName);
        
        if (backupPath != null)
        {
            TerminalUI.WriteFormattedTextByType($"Successfully backed up: {saveName}", "suc", true, false);
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
                _terminalUI.WriteFormattedTextByType($"Error loading save info: {ex.Message}", "err", true, false);
                _terminalUI.WriteFormattedTextByType("No saves found.", "warn", true, false);
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            _terminalUI.WriteFormattedTextByType("No saves found.", "warn", true, false);
            return;
        }
        
        try
        {
            _terminalUI.WriteFormattedTextByType($"\nProcessing {Saves.Count} game entries...", "inf", true, false);
            
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
                _terminalUI.WriteFormattedTextByType($"Error loading save info: {ex.Message}", "err", true, false);
                _terminalUI.WriteFormattedTextByType("No saves found.", "warn", true, false);
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            _terminalUI.WriteFormattedTextByType("No saves found.", "warn", true, false);
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

                _terminalUI.WriteFormattedTextByType($"All Savegames for: {gameName}\n", "suc", true, false);
                
                int index = 0;
                foreach (var save in entry.Value)
                {
                    var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                    _terminalUI.WriteTextWithColor($"[{index}] {displayName}", ConsoleColor.DarkCyan, true, false);
                    index++;
                }
                
                Console.WriteLine(string.Empty);
                _terminalUI.WriteFormattedTextByType("Enter the index of the save file to rename ('cancel' or 'c' to abort): ", "inf", false, false);
                var input = Console.ReadLine();

                if (input.ToLower() is "cancel" or "c")
                {
                    _terminalUI.WriteFormattedTextByType("Operation cancelled.", "inf", true, false);
                    return;
                }

                if (int.TryParse(input, out int saveIndex) && saveIndex >= 0 && saveIndex < entry.Value.Count)
                {
                    _terminalUI.WriteFormattedTextByType($"Current name: {entry.Value[saveIndex].DisplayName}", "inf", true, false);
                    _terminalUI.WriteFormattedTextByType("Enter new display name: ", "inf", false, false);
                    var newName = Console.ReadLine();

                    if (!string.IsNullOrWhiteSpace(newName))
                    {
                        entry.Value[saveIndex].DisplayName = newName;
                        
                        var json = JsonSerializer.Serialize(Saves, new JsonSerializerOptions { WriteIndented = true });
                        File.WriteAllText(_globals.UbiSaveInfoFilePath, json);

                        _terminalUI.WriteFormattedTextByType($"Save file renamed to: {newName}", "suc", true, false);
                    }
                    else
                    {
                        _terminalUI.WriteFormattedTextByType("Invalid name. Operation cancelled.", "err", true, false);
                    }
                }
                else
                {
                    _terminalUI.WriteFormattedTextByType("Invalid selection. Operation cancelled.", "err", true, false);
                }
            }
        }

        if (!gameFound)
        {
            _terminalUI.WriteFormattedTextByType($"The ID: {id} was not found in the detected games list.", "err", true, false);
        }
    }

    public override async Task InitializeSaveDetection()
    {
        try
        {
            _accountFolders = GetAccountId();

            if (_accountFolders.Count == 0)
            {
                _terminalUI.WriteFormattedTextByType(
                    "No Ubisoft accounts found. Unable to proceed with game detection.", "err", true, false);
                return;
            }

            foreach (var accountFolder in _accountFolders)
            {
                var accountId = Path.GetFileName(accountFolder);
                _terminalUI.WriteFormattedTextByType($"Processing account: {accountId}", "inf", true, false);

                await FindGames(accountId);
                await FindSaveGames();
            }
        }
        catch (Exception ex)
        {
            _terminalUI.WriteFormattedTextByType($"Error in InitializeSaveDetection: {ex.Message}", "err", true, false);
            throw;
        }

        _accountFolders = GetAccountId();

        if (_accountFolders.Count == 0)
        {
            _terminalUI.WriteFormattedTextByType("No Ubisoft accounts found. Unable to proceed with game detection.",
                "err", true, false);
            return;
        }

        foreach (var accountFolder in _accountFolders)
        {
            string accountId = Path.GetFileName(accountFolder);
            _terminalUI.WriteFormattedTextByType($"Processing account: {accountId}", "inf", true, false);

            await FindGames(accountId);
            await FindSaveGames();
        }
    }

    private List<string> GetAccountId()
    {
        _terminalUI.WriteFormattedTextByType("Looking for accounts..", "inf", true, false);
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
            _terminalUI.WriteFormattedTextByType("No Ubisoft accounts found!", "err", true, false);
            _configManager.Data.DetectedUbiAccount = string.Empty;
            _configManager.Save();
        }

        // Check if we already have a valid selection
        if (!string.IsNullOrEmpty(_configManager.Data.DetectedUbiAccount))
        {
            if (_configManager.Data.DetectedUbiAccount != "all")
            {
                var existingAccount = Path.Combine(_globals.UbisoftRootFolder, _configManager.Data.DetectedUbiAccount);
                if (Directory.Exists(existingAccount))
                {
                    return new List<string> { existingAccount };
                }
            }

            // If "all" is selected, return all accounts
            return accountFolders;
        }

        if (accountFolders.Count == 1)
        {
            string accountId = Path.GetFileName(accountFolders[0]);
            _terminalUI.WriteFormattedTextByType($"Found Ubisoft account: {accountId}", "suc", true, false);
            _configManager.Data.DetectedUbiAccount = accountId;
            _configManager.Save();
            return accountFolders;
        }

        // Multiple accounts - prompt for selection
        _terminalUI.WriteFormattedTextByType($"Found {accountFolders.Count} Ubisoft accounts: ", "inf", true, false);

        for (int i = 0; i < accountFolders.Count; i++)
        {
            Console.WriteLine($"[{i + 1}] {Path.GetFileName(accountFolders[i])}");
        }

        Console.WriteLine($"[{accountFolders.Count + 1}] All accounts");

        while (true)
        {
            _terminalUI.WriteFormattedTextByType("Select account (enter number): ", "inf", false, false);
            if (int.TryParse(Console.ReadLine(), out int choice) && choice >= 1 && choice <= accountFolders.Count + 1)
            {
                if (choice == accountFolders.Count + 1)
                {
                    _configManager.Data.DetectedUbiAccount = "all";
                    _configManager.Save();
                    return accountFolders;
                }

                var selectedId = Path.GetFileName(accountFolders[choice - 1]);
                _configManager.Data.DetectedUbiAccount = selectedId;
                _configManager.Save();
                return new List<string> { accountFolders[choice - 1] };
            }

            _terminalUI.WriteFormattedTextByType("Invalid selection, try again", "err", true, false);
        }
    }

    private async Task FindGames(string accountId)
    {
        accountRootFolder = Path.Combine(_globals.UbisoftRootFolder, accountId);
        _configManager.Data.DetectedUbiGames.Clear();

        _terminalUI.WriteFormattedTextByType("Looking for games..", "inf", true, false);

        foreach (var gameFolder in Directory.GetDirectories(accountRootFolder))
        {
            var gameId = Path.GetFileName(gameFolder);
            var gameName = await _utilities.TranslateUbisoftGameId(gameFolder);

            _configManager.Data.DetectedUbiGames.Add(gameId);
            _terminalUI.HighlightWordInText($"Game found: {gameName}", ConsoleColor.Yellow, gameName, true, false);
        }

        if (_configManager.Data.DetectedUbiGames.Count == 0)
        {
            _terminalUI.WriteFormattedTextByType("No games found!", "warn", true, false);
        }
        else
        {
            _terminalUI.WriteFormattedTextByType($"Total games found: {_configManager.Data.DetectedUbiGames.Count}\n", "suc", true, false);
        }

        _configManager.Save();
    }

    private async Task FindSaveGames(bool ignoreAutoSave = false)
    {
        _terminalUI.WriteFormattedTextByType("Looking for savegames..", "inf", true, false);

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

            _terminalUI.WriteTextWithColor($"{gameName} - {validSaves.Count} Detected Saves", ConsoleColor.Red, true,
                false);

            foreach (var save in validSaves)
            {
                double sizeInKB = save.FileSize / 1024.0;
                string timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                string timestampCreated = save.DateCreated.ToString("yyyy-MM-dd HH:mm:ss");
                _terminalUI.WriteTextWithColor($"   - {save.FileName} | {sizeInKB:F1}KB | created: {timestampCreated} | updated: {timestamp}", ConsoleColor.DarkCyan, true, false);
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
                _terminalUI.WriteFormattedTextByType($"Error loading save info: {ex.Message}", "err", true, false);
                _terminalUI.WriteFormattedTextByType("No saves found.", "err", true, false);
                return;
            }
        }

        if (Saves == null || Saves.Count == 0)
        {
            _terminalUI.WriteFormattedTextByType("No saves found.", "err", true, false);
            return;
        }

        _terminalUI.WriteFormattedTextByType("Listing all Ubisoft saves:", "inf", true, false);

        foreach (var entry in Saves)
        {
            string[] parts = entry.Key.Split('_');
            var accountId = parts[0];
            var gameId = parts[1];

            var gameName = _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId)).Result;

            _terminalUI.WriteTextWithColor($"\n{gameName} - ", ConsoleColor.DarkCyan, false, false);
            _terminalUI.WriteTextWithColor($"{gameId} - ", ConsoleColor.Yellow, false, false);
            _terminalUI.WriteTextWithColor($"Account: {accountId} - ", ConsoleColor.Red, false, false);
            _terminalUI.WriteTextWithColor($"Total Saves: {entry.Value.Count}", ConsoleColor.White, true, false);

            foreach (var save in entry.Value)
            {
                var sizeInKB = save.FileSize / 1024.0;
                var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                var timestampCreated = save.DateCreated.ToString("yyyy-MM-dd HH:mm:ss");

                var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                _terminalUI.WriteTextWithColor($"   - {displayName} | {sizeInKB:F1}KB | created: {timestampCreated} | modified: {timestamp}", ConsoleColor.Gray, true, false);
            }
        }
    }
}