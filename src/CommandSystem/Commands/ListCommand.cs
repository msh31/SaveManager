//ReSharper disable InconsistentNaming
using SaveManager.Commands;
using SaveManager.Managers;
using Spectre.Console;

class ListCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    
    public ListCommand(UbiManager ubiManager) : base("list", "Shows available saves", "")
    {
        _ubiManager = ubiManager;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        var platform = AnsiConsole.Prompt(
            new SelectionPrompt<string>()
                .Title("Which [green]platform[/] do you want to list saves for?")
                .PageSize(5)
                .AddChoices(new[] {
                    "Ubisoft",
                    "Rockstar"
                }));

        if (platform == "Ubisoft")
        {
            await _ubiManager.ListSaveGamesAsync();
        }
        else
        {
            AnsiConsole.MarkupLine("[cyan][[inf]][/] Rockstar list not implemented yet.");
        }
    }
}