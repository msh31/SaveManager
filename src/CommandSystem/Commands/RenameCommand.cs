
using System.Text.Json;
using SaveManager;
using SaveManager.Commands;
using SaveManager.Models;
using Spectre.Console;

class RenameCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    private readonly Globals _globals;
    private readonly Utilities _utilities;
    
    public RenameCommand(UbiManager ubiManager, Globals globals, Utilities utilities) : base("rename", "Rename a save file's display name", "[gameId] [saveFileName] [newDisplayName]") 
    {
        _ubiManager = ubiManager;
        _globals = globals;
        _utilities = utilities;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        try
        {
            Dictionary<string, List<SaveFileInfo>> saves = new();
            if (File.Exists(_globals.UbiSaveInfoFilePath))
            {
                try
                {
                    var json = await File.ReadAllTextAsync(_globals.UbiSaveInfoFilePath);
                    var options = new JsonSerializerOptions { 
                        WriteIndented = true,
                        TypeInfoResolver = JsonContext.Default 
                    };
                    saves = JsonSerializer.Deserialize<Dictionary<string, List<SaveFileInfo>>>(json, options);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error loading save info: {ex.Message}");
                    return;
                }
            }

            if (saves == null || saves.Count == 0)
            {
                Console.WriteLine("No saves found. Try running 'refresh ubisoft' first.");
                return;
            }
            
            if (args.Length >= 3)
            {
                string gameId = args[0];
                string saveFileName = args[1];
                string displayNameArg = string.Join(" ", args.Skip(2));
                
                await _ubiManager.RenameSaveFilesAsync(gameId, saveFileName, displayNameArg);
                return;
            }

            var gameOptions = new List<(string GameId, string AccountId, string GameName, string DisplayChoice)>();
            
            foreach (var entry in saves)
            {
                string[] parts = entry.Key.Split('_');
                if (parts.Length < 2) continue;
                
                var accountId = parts[0];
                var gameId = parts[1];
                
                var gameName = await _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, accountId, gameId));

                var displayChoice = $"{gameName} - {gameId}";
                
                gameOptions.Add((gameId, accountId, gameName, displayChoice));
            }
            
            if (gameOptions.Count == 0)
            {
                Console.WriteLine("No games with saves found.");
                return;
            }

            var gameChoices = gameOptions.Select(g => g.DisplayChoice).ToList();
            
            var gamePrompt = new SelectionPrompt<string>()
                .Title("Select a game to rename saves for:")
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more games)[/]")
                .AddChoices(gameChoices);
            
            var selectedGameChoice = AnsiConsole.Prompt(gamePrompt);
            var selectedGame = gameOptions.First(g => g.DisplayChoice == selectedGameChoice);
            var saveKey = $"{selectedGame.AccountId}_{selectedGame.GameId}";
            
            if (!saves.TryGetValue(saveKey, out var saveFiles) || saveFiles.Count == 0)
            {
                Console.WriteLine("No save files found for this game.");
                return;
            }

            var saveOptions = new List<(string DisplayChoice, SaveFileInfo Save)>();
            
            foreach (var save in saveFiles)
            {
                var displayName = save.DisplayName == "CUSTOM_NAME_NOT_SET" ? save.FileName : save.DisplayName;
                var safeDisplayName = EscapeSpecialCharacters(displayName);
                var safeFileName = EscapeSpecialCharacters(save.FileName);
                var displayChoice = $"{safeDisplayName} - {safeFileName}";
                
                saveOptions.Add((displayChoice, save));
            }
            
            var saveChoices = saveOptions.Select(s => s.DisplayChoice).ToList();

            var savePrompt = new SelectionPrompt<string>()
                .Title("Select a save file to rename:")
                .PageSize(10)
                .MoreChoicesText("(Move up and down to reveal more saves)")
                .AddChoices(saveChoices);
            
            var selectedSaveChoice = AnsiConsole.Prompt(savePrompt);
            var selectedSaveOption = saveOptions.FirstOrDefault(s => s.DisplayChoice == selectedSaveChoice);
            
            if (selectedSaveOption == default)
            {
                AnsiConsole.MarkupLine("[red][[err]][/] Failed to find the selected save. This might be due to special characters in filenames.");
                return;
            }
            
            var selectedSave = selectedSaveOption.Save;
            var currentDisplayName = selectedSave.DisplayName == "CUSTOM_NAME_NOT_SET" ? selectedSave.FileName : selectedSave.DisplayName;
            
            Console.WriteLine($"Current display name: {currentDisplayName}");

            var escapedDisplayName = Markup.Escape(currentDisplayName);
            var newDisplayName = AnsiConsole.Ask("Enter new display name:", escapedDisplayName);

            var escapedCurrentName = Markup.Escape(currentDisplayName);
            var escapedNewName = Markup.Escape(newDisplayName);
            var confirm = AnsiConsole.Confirm($"Rename from '{escapedCurrentName}' to '{escapedNewName}'?", true);
            
            if (confirm)
            {
                await AnsiConsole.Status()
                    .StartAsync("Renaming save...", async ctx => {
                        ctx.Spinner(Spinner.Known.Dots);
                        ctx.SpinnerStyle = new Style(Color.Green);
                        
                        await _ubiManager.RenameSaveFilesAsync(selectedGame.GameId, selectedSave.FileName, newDisplayName);
                        await Task.Delay(500);
                    });
                
                AnsiConsole.WriteLine("Save renamed successfully!");
            }
            else
            {
                Console.WriteLine("Rename operation cancelled.");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error during rename operation: {ex.Message}");
            Console.WriteLine(ex.StackTrace);
        }
    }
    

    private string EscapeSpecialCharacters(string text)
    {
        if (string.IsNullOrEmpty(text))
        {
            return text;
        }
        
        return Markup.Escape(text);
    }
}