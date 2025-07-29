// ReSharper disable FunctionNeverReturns (command handler handles exit)
using SaveManager.CommandSystem;
using SaveManager.Managers;
using SaveManager.Services;
using SaveManager.Utilities;
using Spectre.Console;

class CommandLoop
{
    private readonly CommandHandler _commandHandler;

    public CommandLoop(SaveManager.Utilities.SaveManager saveManager, Globals globals, Utilities utilities, GameArtwork artwork) {
        _commandHandler = new CommandHandler(saveManager, globals, utilities, artwork);
    }

    public async Task StartAsync()
    {
        while (true) {
            try {
                AnsiConsole.Markup("[green]> [/]");
                var input = Console.ReadLine();

                if (string.IsNullOrEmpty(input)) {
                    continue;
                }

                await _commandHandler.ExecuteAsync(input);
            }
            catch (Exception ex) {
                AnsiConsole.MarkupLine($"[red]Error in command loop:[/] {ex.Message}");
            }
        }
    }
}