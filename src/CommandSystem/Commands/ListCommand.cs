//ReSharper disable InconsistentNaming
using SaveManager.Commands;
using SaveManager.Managers;
using Spectre.Console;

class ListCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    
    public ListCommand(UbiManager ubiManager) : base("list", "Shows available saves", "[ubisoft|u|rockstar|r]")
    {
        _ubiManager = ubiManager;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        var fruits = AnsiConsole.Prompt(
            new MultiSelectionPrompt<string>()
                .Title("Which [green]games[/] do you want to backup?")
                .NotRequired() // Not required to have a favorite fruit
                .PageSize(10)
                .MoreChoicesText("[grey](Move up and down to reveal more games)[/]")
                .InstructionsText(
                    "[grey](Press [blue]<space>[/] to toggle a game, " + 
                    "[green]<enter>[/] to accept)[/]")
                .AddChoices(new[] {
                    "Apple", "Apricot", "Avocado",
                    "Banana", "Blackcurrant", "Blueberry",
                    "Cherry", "Cloudberry", "Coconut",
                }));

// Write the selected fruits to the terminal
        foreach (string fruit in fruits)
        {
            AnsiConsole.WriteLine(fruit);
        }
        
        // if (args.Length > 0)
        // {
        //     var platform = args[0].ToLower();
        //     
        //     if (platform is "ubisoft" or "u")
        //     {
        //         await _ubiManager.ListSaveGamesAsync();
        //     }
        //     else if (platform is "rockstar" or "r")
        //     {
        //         _terminalUI.WriteFormattedTextByType("Rockstar list not implemented yet.", "inf", true, false);
        //     }
        //     else
        //     {
        //         _terminalUI.WriteFormattedTextByType($"Unknown platform: {platform}. Use 'ubisoft' or 'rockstar'", "err", true, false);
        //     }
        // }
        // else
        // {
        //     _terminalUI.WriteFormattedTextByType("Please specify platform: 'list ubisoft' or 'list rockstar'", "inf", true, false);
        // }
    }
}