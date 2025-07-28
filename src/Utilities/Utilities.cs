//ReSharper disable InconsistentNaming

using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using System.Text.Json;
using SaveManager.Managers;
using Spectre.Console;

namespace SaveManager.Utilities;

public class Utilities(Logger logger)
{
    private ConcurrentDictionary<string, string> gameNames;
    private readonly HttpClient client = new();
    bool areUbisoftTranslationsLoaded;
    
    public async Task<string> TranslateUbisoftGameId(string gameFolder)
    {
        if (!areUbisoftTranslationsLoaded) {
            await LoadUbisoftGameTranslations();
            areUbisoftTranslationsLoaded = true;
        }

        var gameId = Path.GetFileName(gameFolder);

        if (gameNames.TryGetValue(gameId, out string fullName)) {
            return fullName;
        }

        return gameId;
    }
    
    private async Task LoadUbisoftGameTranslations()
    {
        try { 
            var json = await client.GetStringAsync("https://raw.githubusercontent.com/msh31/Ubisoft-Game-Ids/refs/heads/master/gameids.json");
            var franchises = JsonSerializer.Deserialize<Dictionary<string, Dictionary<string, string>>>(json);

            if (franchises != null)
            {
                gameNames = new ConcurrentDictionary<string, string>();
                
                foreach (var franchise in franchises.Values) {
                    foreach (var game in franchise) {
                        gameNames[game.Key] = game.Value;
                    }
                }
            }
        } catch (Exception ex) {
            logger.Fatal(ex.Message, 2, true);
            gameNames = new ConcurrentDictionary<string, string>();
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
            if (Path.GetFileName(folder).Length > 20) {
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
            if (configManager.Data.DetectedUbiAccount == "all") {
                return accountFolders;
            }

            var existingAccount = Path.Combine(globals.UbisoftRootFolder,  configManager.Data.DetectedUbiAccount);

            if (Directory.Exists(existingAccount)) {
                return new List<string> { existingAccount };
            }
        }

        if (accountFolders.Count == 1) {
            string accountId = Path.GetFileName(accountFolders[0]);
            AnsiConsole.MarkupLine($"[green][[suc]][/] Found Ubisoft account: {accountId}");
            configManager.Data.DetectedUbiAccount = accountId;
            configManager.Save();
            
            return accountFolders;
        }

        AnsiConsole.MarkupLine($"[cyan][[inf]][/] Found {accountFolders.Count} Ubisoft accounts:");
        
        var choices = new List<string>();

        foreach (var folder in accountFolders) {
            choices.Add(Path.GetFileName(folder));
        }

        choices.Add("All accounts");

        var selectedOption = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Which [yellow]account[/] do you want to use?")
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more accounts)[/]")
                .AddChoices(choices));
        
        if (selectedOption == "All accounts") {
             configManager.Data.DetectedUbiAccount = "all";
             configManager.Save();
             
            return accountFolders;
        }

        for (int i = 0; i < accountFolders.Count; i++) {
            if (Path.GetFileName(accountFolders[i]) == selectedOption) {
                 configManager.Data.DetectedUbiAccount = selectedOption;
                 configManager.Save();
                 
                return new List<string> { accountFolders[i] };
            }
        }
        
        AnsiConsole.MarkupLine("[red][[err]][/] Something went wrong! [error code: 0x0896]");
        return new List<string>();
    }
}