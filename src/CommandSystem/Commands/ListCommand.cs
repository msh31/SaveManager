//ReSharper disable InconsistentNaming
using SaveManager.Commands;
using SaveManager.Managers;
using SaveManager.Models;
using Spectre.Console;
using System.Text.Json;
using SaveManager;

class ListCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    //private readonly RockstarManager _rockstarManager;
    private readonly Utilities _utilities;
    private readonly Globals _globals;
    
    public ListCommand(UbiManager ubiManager, Utilities utilities, Globals globals/*, RockstarManager rockstarManager*/) : base("list", "Shows available saves", "")
    {
        _ubiManager = ubiManager;
        //_rockstarManager = rockstarManager;
        _utilities = utilities;
        _globals = globals;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        try
        {
            var platform = AnsiConsole.Prompt(
                new SelectionPrompt<string>()
                    .Title("Which [green]platform[/] do you want to list saves for?")
                    .PageSize(5)
                    .MoreChoicesText("[grey](Move up and down to see more options)[/]")
                    .AddChoices(new[] {
                        "Ubisoft",
                        "Rockstar"
                    }));

            var listOption = AnsiConsole.Prompt(
                new SelectionPrompt<string>()
                    .Title("How would you like to [green]list[/] the saves?")
                    .PageSize(5)
                    .MoreChoicesText("[grey](Move up and down to see more options)[/]")
                    .AddChoices(new[] {
                        "All games",
                        "By franchise",
                        "Specific game"
                    }));

            switch (listOption)
            {
                case "All games":
                    await ListAllGames(platform.ToLower());
                    break;
                case "By franchise":
                    await ListByFranchise(platform.ToLower());
                    break;
                case "Specific game":
                    await PromptAndListSpecificGame();
                    break;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error executing list command: {ex.Message}");
        }
    }

    private async Task PromptAndListSpecificGame()
    {
        try
        {
            var savesData = await LoadSavesData();
            
            if (savesData == null || savesData.Count == 0)
            {
                AnsiConsole.Write(new Markup("[red][[err]][/] no saves found."));
                return;
            }

            var gameOptions = new List<string>();
            var gameIdMap = new Dictionary<string, string>();

            foreach (var entry in savesData)
            {
                string[] parts = entry.Key.Split('_');
                var accountId = parts[0];
                var gameId = parts[1];

                var gameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));
                var safeGameName = Markup.Escape(gameName);
                var displayName = $"{safeGameName} ({gameId})";
                
                gameOptions.Add(displayName);
                gameIdMap[displayName] = gameId;
            }

            if (gameOptions.Count == 0)
            {
                Console.WriteLine("No games found with saves.");
                return;
            }

            var selectedGame = AnsiConsole.Prompt(
                new SelectionPrompt<string>()
                    .Title("Select a [green]game[/]:")
                    .PageSize(10)
                    .MoreChoicesText("[grey](Move up and down to see more games)[/]")
                    .AddChoices(gameOptions));

            var selectedGameId = gameIdMap[selectedGame];
            await ListUbisoftGameSaves(selectedGameId);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error when prompting for game: {ex.Message}");
        }
    }

    // private async Task ListSavesForPlatform(string platform, string gameId)
    // {
    //     try
    //     {
    //         await AnsiConsole.Status()
    //             .StartAsync("Loading save games...", async ctx => 
    //             {
    //                 ctx.Spinner(Spinner.Known.Dots);
    //                 ctx.SpinnerStyle = new Style(Color.Green);
    //                 
    //                 await Task.Delay(500);
    //             });
    //
    //         if (platform is "ubisoft" or "u")
    //         {
    //             if (gameId != null)
    //             {
    //                 await ListUbisoftGameSaves(gameId);
    //             }
    //             else
    //             {
    //                 await _ubiManager.ListSaveGamesAsync();
    //             }
    //         }
    //         else if (platform is "rockstar" or "r")
    //         {
    //             Console.WriteLine("Rockstar list feature is not implemented yet.");
    //         }
    //         else
    //         {
    //             Console.WriteLine($"Unknown platform: '{platform}'. Use 'ubisoft' or 'rockstar'.");
    //         }
    //     }
    //     catch (Exception ex)
    //     {
    //         Console.WriteLine($"Error listing saves: {ex.Message}");
    //     }
    // }

    private async Task ListAllGames(string platform)
    {
        try
        {
            if (platform is not ("ubisoft" or "u"))
            {
                Console.WriteLine($"Platform '{platform}' not supported for listing all games.");
                return;
            }
            
            // if (platform is ("r"))
            // {
            //     await _rockstarManager.InitializeSaveDetection();
            // }

            await AnsiConsole.Status()
                .StartAsync("Loading all games...", async ctx => 
                {
                    ctx.Spinner(Spinner.Known.Dots);
                    ctx.SpinnerStyle = new Style(Color.Green);
                    await Task.Delay(500);
                });

            var savesData = await LoadSavesData();
            if (savesData == null || savesData.Count == 0)
            {
                Console.WriteLine("No saves found.");
                return;
            }

            var gameTable = new Table()
                .Title("All Games with Saves")
                .Border(TableBorder.Rounded)
                .AddColumn(new TableColumn("Game").LeftAligned())
                .AddColumn(new TableColumn("Game ID").Centered())
                .AddColumn(new TableColumn("Account").Centered())
                .AddColumn(new TableColumn("Saves").Centered())
                .AddColumn(new TableColumn("Latest Save").Centered())
                .Expand();

            var sortedEntries = new List<(string GameName, string GameId, string AccountId, int SaveCount, DateTime LatestSave)>();

            foreach (var entry in savesData)
            {
                string[] parts = entry.Key.Split('_');
                var accountId = parts[0];
                var gameId = parts[1];

                var gameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));
                var safeGameName = Markup.Escape(gameName);
                var saveCount = entry.Value.Count;
                var latestSave = entry.Value.Count > 0 ? 
                    entry.Value.Max(s => s.LastModified) : 
                    DateTime.MinValue;

                sortedEntries.Add((safeGameName, gameId, accountId, saveCount, latestSave));
            }
            
            sortedEntries = sortedEntries.OrderBy(e => e.GameName).ToList();

            foreach (var entry in sortedEntries)
            {
                gameTable.AddRow(
                    entry.GameName,
                    entry.GameId,
                    entry.AccountId.Substring(0, 6) + "...",
                    entry.SaveCount.ToString(),
                    entry.LatestSave.ToString("yyyy-MM-dd HH:mm")
                );
            }

            AnsiConsole.Write(gameTable);
            Console.WriteLine("\nTIP: To view saves for a specific game, use: list ubisoft <gameId>");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error listing all games: {ex.Message}");
        }
    }

    private async Task ListByFranchise(string platform)
    {
        try
        {
            if (platform is not ("ubisoft" or "u"))
            {
                Console.WriteLine($"Platform '{platform}' not supported for listing by franchise.");
                return;
            }

            await AnsiConsole.Status()
                .StartAsync("Organizing games by franchise...", async ctx => 
                {
                    ctx.Spinner(Spinner.Known.Dots);
                    ctx.SpinnerStyle = new Style(Color.Green);
                    await Task.Delay(500);
                });

            var savesData = await LoadSavesData();
            if (savesData == null || savesData.Count == 0)
            {
                Console.WriteLine("No saves found.");
                return;
            }
            
            var franchises = new Dictionary<string, List<(string GameName, string GameId, string AccountId, int SaveCount)>>();

            foreach (var entry in savesData)
            {
                string[] parts = entry.Key.Split('_');
                var accountId = parts[0];
                var gameId = parts[1];

                var fullGameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));
                var safeGameName = Markup.Escape(fullGameName);
                var saveCount = entry.Value.Count;
                
                var franchiseName = ExtractFranchise(fullGameName);
                var safeFranchiseName = Markup.Escape(franchiseName);
                
                if (!franchises.ContainsKey(safeFranchiseName))
                {
                    franchises[safeFranchiseName] = new List<(string, string, string, int)>();
                }

                franchises[safeFranchiseName].Add((safeGameName, gameId, accountId, saveCount));
            }
            
            var sortedFranchises = franchises.Keys.OrderBy(f => f).ToList();

            foreach (var franchise in sortedFranchises)
            {
                var franchisePanel = new Panel($"[bold]{franchise}[/] ({franchises[franchise].Count} games)")
                {
                    Border = BoxBorder.Rounded,
                    Padding = new Padding(1, 0, 1, 0)
                };

                AnsiConsole.Write(franchisePanel);

                var gamesTable = new Table()
                    .Border(TableBorder.Rounded)
                    .AddColumn(new TableColumn("Game").LeftAligned())
                    .AddColumn(new TableColumn("Game ID").Centered())
                    .AddColumn(new TableColumn("Saves").Centered())
                    .Expand();

                foreach (var game in franchises[franchise].OrderBy(g => g.GameName))
                {
                    gamesTable.AddRow(
                        game.GameName,
                        game.GameId,
                        game.SaveCount.ToString()
                    );
                }

                AnsiConsole.Write(gamesTable);
                Console.WriteLine();
            }

            Console.WriteLine("\nTIP: To view saves for a specific game, use: list ubisoft <gameId>");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error listing by franchise: {ex.Message}");
        }
    }

    private string ExtractFranchise(string gameName)
    {
        string[] knownFranchises = {
            "Assassin's Creed", "Far Cry", "Watch_Dogs", "Watch Dogs", 
            "Tom Clancy's", "Prince of Persia", "Rayman", "The Crew", 
            "Anno", "For Honor", "Just Dance", "Trials"
        };

        foreach (var franchise in knownFranchises)
        {
            if (gameName.StartsWith(franchise, StringComparison.OrdinalIgnoreCase))
            {
                return franchise;
            }
        }
        
        var words = gameName.Split(' ');
        if (words.Length == 1) 
            return gameName;  
        
        if (gameName.Contains(':'))
            return gameName.Split(':')[0].Trim();
        
        if (words.Length >= 2)
            return $"{words[0]} {words[1]}";
            
        return "Other";
    }

    private async Task ListUbisoftGameSaves(string gameId)
    {
        try
        {
            var savesData = await LoadSavesData();
            if (savesData == null || savesData.Count == 0)
            {
                Console.WriteLine("No saves found.");
                return;
            }

            bool gameFound = false;
            
            foreach (var entry in savesData)
            {
                string[] parts = entry.Key.Split('_');
                var entryGameId = parts[1];

                if (entryGameId.Equals(gameId, StringComparison.OrdinalIgnoreCase))
                {
                    gameFound = true;
                    var accountId = parts[0];
                    var gameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));
                    
                    var safeGameName = Markup.Escape(gameName);
                    
                    var table = new Table();
                    table.Title = new TableTitle($"{safeGameName} | {gameId}");
                    table.Border(TableBorder.Rounded);
                    table.Expand();
                    
                    table.AddColumn(new TableColumn("Index").Centered());
                    table.AddColumn(new TableColumn("Name").LeftAligned());
                    table.AddColumn(new TableColumn("Size").RightAligned());
                    table.AddColumn(new TableColumn("Created").Centered());
                    table.AddColumn(new TableColumn("Modified").Centered());

                    for (int i = 0; i < entry.Value.Count; i++)
                    {
                        var save = entry.Value[i];
                        var rawDisplayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                        
                        var safeDisplayName = Markup.Escape(rawDisplayName);
                        
                        var sizeInKB = save.FileSize / 1024.0;
                        
                        table.AddRow(
                            $"{i}",
                            safeDisplayName,
                            $"{sizeInKB:F1}KB",
                            save.DateCreated.ToString("yyyy-MM-dd HH:mm"),
                            save.LastModified.ToString("yyyy-MM-dd HH:mm")
                        );
                    }

                    AnsiConsole.Write(table);
                    Console.WriteLine("\nTIP: To backup a save, use: backup create [gameId] [index]");
                }
            }

            if (!gameFound)
            {
                Console.WriteLine($"Game with ID '{gameId}' not found.");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error processing game saves: {ex.Message}");
        }
    }

    private async Task<Dictionary<string, List<SaveFileInfo>>> LoadSavesData()
    {
        try
        {
            if (File.Exists(_globals.UbiSaveInfoFilePath))
            {
                var json = await File.ReadAllTextAsync(_globals.UbiSaveInfoFilePath);
                var options = new JsonSerializerOptions { 
                    WriteIndented = true,
                    TypeInfoResolver = JsonContext.Default 
                };
                return JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json, options);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error loading save info: {ex.Message}");
        }
    
        return new Dictionary<string, List<SaveFileInfo>>();
    }
}