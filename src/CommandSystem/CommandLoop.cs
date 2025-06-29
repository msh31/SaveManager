using Spectre.Console;

class CommandLoop
{
    private readonly CommandHandler commandHandler;

    public CommandLoop(UbiManager ubiManager)
    {
        commandHandler = new CommandHandler(ubiManager);
    }

    public async Task StartAsync()
    {
        while (true)
        {
            try
            {
                AnsiConsole.Markup("[green]> [/]");
                var input = Console.ReadLine();

                if (string.IsNullOrEmpty(input))
                    continue;

                await commandHandler.ExecuteAsync(input);
            }
            catch (Exception ex)
            {
                AnsiConsole.MarkupLine($"[red]Error in command loop:[/] {ex.Message}");
            }
        }
    }
}