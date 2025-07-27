// ReSharper disable FunctionNeverReturns (command handler handles exit)
using SaveManager.CommandSystem;
using SaveManager.Managers;
using SaveManager.Utilities;
using Spectre.Console;

class CommandLoop
{
    private readonly CommandHandler commandHandler;

    public CommandLoop(SaveManager.Utilities.SaveManager saveManager) {
        commandHandler = new CommandHandler(saveManager);
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

                await commandHandler.ExecuteAsync(input);
            }
            catch (Exception ex) {
                AnsiConsole.MarkupLine($"[red]Error in command loop:[/] {ex.Message}");
            }
        }
    }
}