//ReSharper disable InconsistentNaming

using System.Text.Json;
using SaveManager.Managers;
using SaveManager.Models;

class UbiManager : BaseManager
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
    
    public async Task InitializeSaveDetection()
    {
        try
        {
            _accountFolders = GetAccountId();
            
            if (_accountFolders.Count == 0)
            {
                _terminalUI.WriteFormattedTextByType("No Ubisoft accounts found. Unable to proceed with game detection.", "err", true, false);
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
            _terminalUI.WriteFormattedTextByType("No Ubisoft accounts found. Unable to proceed with game detection.", "err", true, false);
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
    
    private async Task FindSaveGames()
    {
        _terminalUI.WriteFormattedTextByType("Looking for savegames..", "inf", true, false);
        
        foreach (var gameId in _configManager.Data.DetectedUbiGames)
        {
            var gameFolder = Path.Combine(accountRootFolder, gameId);
            
            if (!Directory.Exists(gameFolder)) { continue; }

            var gameName = await _utilities.TranslateUbisoftGameId(gameFolder);
            var files = Directory.GetFiles(gameFolder);
            var validSaves = new List<SaveFileInfo>();

            foreach (var file in files)
            {
                var fileName = Path.GetFileName(file);
                
                // ignore these files
                if (fileName.IndexOf("[Options]", StringComparison.OrdinalIgnoreCase) >= 0) { continue; }
                if (!fileName.EndsWith(".save", StringComparison.OrdinalIgnoreCase)) { continue; }

                var fileInfo = new FileInfo(file);
                
                // string displayName = fileName;
                // if (fileName.EndsWith(".save", StringComparison.OrdinalIgnoreCase))
                // {
                //     displayName = fileName.Substring(0, fileName.Length - 5);
                // }

                var saveInfo = new SaveFileInfo
                {
                    FileName = fileName,
                    FileSize = fileInfo.Length / 1024,
                    LastModified = fileInfo.LastWriteTime,
                    DisplayName = "CUSTOM_NAME_NOT_SET"
                };

                validSaves.Add(saveInfo);
            }

            if (!validSaves.Any()) { continue; }
            
            var saveKey = $"{Path.GetFileName(accountRootFolder)}_{gameId}";
            Saves[saveKey] = validSaves;
            var json = JsonSerializer.Serialize(Saves, new JsonSerializerOptions { WriteIndented = true });
            await File.WriteAllTextAsync(_globals.UbiSaveInfoFilePath, json);
            
            _terminalUI.WriteTextWithColor($"{gameName} - {validSaves.Count} Detected Saves", ConsoleColor.Red, true, false);
            
            foreach (var save in validSaves)
            {
                double sizeInKB = save.FileSize / 1024.0;
                string timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                _terminalUI.WriteTextWithColor($"   - {save.FileName} | {sizeInKB:F1}KB | {timestamp}", ConsoleColor.DarkCyan, true, false);
            }
        }
    }
    
    public void ListSaveGames()
    {
        if (Saves.Count == 0 && File.Exists(_globals.UbiSaveInfoFilePath))
        {
            try
            {
                string json = File.ReadAllText(_globals.UbiSaveInfoFilePath);
                Saves = JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json);
            }
            catch (Exception ex)
            {
                _terminalUI.WriteFormattedTextByType($"Error loading save info: {ex.Message}", "err", true, false);
                _terminalUI.WriteFormattedTextByType("No saves found. Run the detection first.", "warn", true, false);
                return;
            }
        }
        
        if (Saves == null || Saves.Count == 0)
        {
            _terminalUI.WriteFormattedTextByType("No saves found. Run the detection first.", "warn", true, false);
            return;
        }
        
        _terminalUI.WriteFormattedTextByType("Listing all Ubisoft saves:", "inf", true, false);
        
        foreach (var entry in Saves)
        {
            string[] parts = entry.Key.Split('_');
            var accountId = parts[0];
            var gameId = parts[1];
            
            var gameName = _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId)).Result;
            
            _terminalUI.WriteTextWithColor($"\n{gameName} ({gameId}) - Account: {accountId}", ConsoleColor.Yellow, true, false);
            _terminalUI.WriteTextWithColor($"Total Saves: {entry.Value.Count}", ConsoleColor.White, true, false);
            
            foreach (var save in entry.Value)
            {
                var sizeInKB = save.FileSize / 1024.0;
                var timestamp = save.LastModified.ToString("yyyy-MM-dd HH:mm:ss");
                
                var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                _terminalUI.WriteTextWithColor($"   - {displayName} | {sizeInKB:F1}KB | {timestamp}", ConsoleColor.Cyan, true, false);
            }
        }
    }
}