//ReSharper disable InconsistentNaming


using System.Text.Json;
using SaveManager.Managers;
using Spectre.Console;

namespace SaveManager.Utilities;

public class Utilities(Logger logger)
{
    private Dictionary<string, string> gameNames = new();
    private Dictionary<string, string> gameNameCache = new();
    private readonly HttpClient client = new();
    bool areUbisoftTranslationsLoaded;    
    
    public async Task<string> TranslateUbisoftGameId(string gameFolder)
    {
        var gameId = Path.GetFileName(gameFolder);
        if (string.IsNullOrEmpty(gameId))
        {
            gameId = string.Empty;
        }        
        if (gameNameCache.ContainsKey(gameId)) {
            return gameNameCache[gameId];
        }
        
        if (!areUbisoftTranslationsLoaded) {
            await LoadUbisoftGameTranslations();
            areUbisoftTranslationsLoaded = true;
        }

        if (gameNames.ContainsKey(gameId)) {
            gameNameCache[gameId] = gameNames[gameId];
            return gameNames[gameId];
        }
        
        gameNameCache[gameId] = gameId;
        return gameId;
    }    
    private async Task LoadUbisoftGameTranslations()
    {
        try { 
            var json = await client.GetStringAsync("https://raw.githubusercontent.com/msh31/Ubisoft-Game-Ids/refs/heads/master/gameids.json");
            var franchises = JsonSerializer.Deserialize<Dictionary<string, Dictionary<string, string>>>(json);

            if (franchises != null)
            {
                gameNames = new Dictionary<string, string>();
                
                foreach (var franchise in franchises.Values) {
                    foreach (var game in franchise) {
                        gameNames[game.Key] = game.Value;
                    }
                }
            }        } catch (Exception ex) {
            logger.Fatal(ex.Message, 2, true);
            gameNames = new Dictionary<string, string>();
        }
    }
    
    public List<string> GetAccountId(Globals globals, ConfigManager configManager)
    {
        if (string.IsNullOrWhiteSpace(globals.UbisoftRootFolder) || !Directory.Exists(globals.UbisoftRootFolder)) {
            throw new InvalidOperationException("Ubisoft savegame folder could not be found or is invalid.");
        }
        
        AnsiConsole.MarkupLine("[cyan][[inf]][/] Looking for accounts..");
        var accountFolders = new List<string>();

        foreach (var folder in Directory.GetDirectories(globals.UbisoftRootFolder)) {
            var folderName = Path.GetFileName(folder);
            if (!string.IsNullOrEmpty(folderName) && folderName.Length > Globals.MinimumAccountIdLength) {
                accountFolders.Add(folder);
            }
        }
        if (accountFolders.Count == 0)
        {
            AnsiConsole.MarkupLine("[red][[err]][/] No Ubisoft accounts found!");
            configManager.Data.DetectedUbiAccount = string.Empty;
            configManager.Save();
            
            return accountFolders;
        }

        if (!string.IsNullOrEmpty( configManager.Data.DetectedUbiAccount))
        {
            if (configManager.Data.DetectedUbiAccount == Globals.AllAccountsOption) {
                return accountFolders;
            }

            var existingAccount = Path.Combine(globals.UbisoftRootFolder,  configManager.Data.DetectedUbiAccount);

            if (Directory.Exists(existingAccount)) {
                return new List<string> { existingAccount };
            }
        }

        if (accountFolders.Count == 1) {
            string accountId = Path.GetFileName(accountFolders[0]);
            if (string.IsNullOrEmpty(accountId))
            {
                accountId = string.Empty;
            }            AnsiConsole.MarkupLine($"[green][[suc]][/] Found Ubisoft account: {accountId}");
            configManager.Data.DetectedUbiAccount = accountId;
            configManager.Save();
            
            return accountFolders;
        }

        AnsiConsole.MarkupLine($"[cyan][[inf]][/] Found {accountFolders.Count} Ubisoft accounts:");
        
        var choices = new List<string>();

        foreach (var folder in accountFolders) {
            var folderName = Path.GetFileName(folder);
            choices.Add(string.IsNullOrEmpty(folderName) ? string.Empty : folderName);
        }
        choices.Add("All accounts");

        var selectedOption = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Which [yellow]account[/] do you want to use?")
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more accounts)[/]")
                .AddChoices(choices));
        
        if (selectedOption == "All accounts") {
             configManager.Data.DetectedUbiAccount = Globals.AllAccountsOption;
             configManager.Save();
             
            return accountFolders;
        }

        for (int i = 0; i < accountFolders.Count; i++) {
            var folderName = Path.GetFileName(accountFolders[i]);
            if ((string.IsNullOrEmpty(folderName) ? string.Empty : folderName) == selectedOption) {                 configManager.Data.DetectedUbiAccount = selectedOption;
                 configManager.Save();
                 
                return new List<string> { accountFolders[i] };
            }
        }
        
        AnsiConsole.MarkupLine("[red][[err]][/] Something went wrong! [error code: 0x0896]");
        return new List<string>();
    }
}