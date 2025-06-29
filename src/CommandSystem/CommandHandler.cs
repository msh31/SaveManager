using Spectre.Console;

class CommandHandler
{
    private readonly UbiManager ubiManager;

    public CommandHandler(UbiManager ubiManager)
    {
        this.ubiManager = ubiManager;
    }

    public async Task ExecuteAsync(string input)
    {
        if (string.IsNullOrWhiteSpace(input))
            return;

        var parts = input.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);
        var command = parts[0].ToLower();
        var args = parts.Length > 1 ? parts[1..] : Array.Empty<string>();

        try
        {
            switch (command)
            {
                case "help" or "h":
                    ShowHelp(args);
                    break;

                case "list" or "l":
                    await ListSaves();
                    break;

                case "rename" or "r":
                    await RenameSave(args);
                    break;

                case "refresh":
                    await RefreshSaves();
                    break;

                case "sync" or "s":
                    await SyncSaves();
                    break;

                case "clear" or "cls":
                    ClearScreen();
                    break;

                case "exit" or "quit" or "q":
                    ExitApp();
                    break;

                default:
                    ShowUnknownCommand(command);
                    break;
            }
        }
        catch (Exception ex)
        {
            AnsiConsole.MarkupLine($"[red]Error executing command:[/] {ex.Message}");
        }
    }

    private void ShowHelp(string[] args)
    {
        if (args.Length > 0)
        {
            ShowSpecificHelp(args[0]);
            return;
        }

        AnsiConsole.MarkupLine("[cyan]Available commands:[/]");
        AnsiConsole.MarkupLine("  [yellow]list[/] (l)     - List all save games");
        AnsiConsole.MarkupLine("  [yellow]rename[/] (r)   - Rename a save file");
        AnsiConsole.MarkupLine("  [yellow]refresh[/]      - Refresh save games list");
        AnsiConsole.MarkupLine("  [yellow]sync[/] (s)     - Sync saves between platforms");
        AnsiConsole.MarkupLine("  [yellow]clear[/] (cls)  - Clear the console");
        AnsiConsole.MarkupLine("  [yellow]exit[/] (q)     - Exit the application");
        AnsiConsole.MarkupLine("  [yellow]help[/] (h)     - Show this help");
        AnsiConsole.MarkupLine("\n[cyan]Tip:[/] Type [yellow]'help <command>'[/] for detailed usage.");
    }

    private void ShowSpecificHelp(string command)
    {
        switch (command.ToLower())
        {
            case "list" or "l":
                AnsiConsole.MarkupLine("[cyan]list[/] - List all save games");
                AnsiConsole.MarkupLine("Shows an interactive list of all detected save games organized by game and platform.");
                break;

            case "rename" or "r":
                AnsiConsole.MarkupLine("[cyan]rename[/] - Rename a save file");
                AnsiConsole.MarkupLine("Usage: [yellow]rename [gameId] [fileName] [newName][/]");
                AnsiConsole.MarkupLine("If no arguments provided, starts interactive rename process.");
                break;

            case "refresh":
                AnsiConsole.MarkupLine("[cyan]refresh[/] - Refresh save games list");
                AnsiConsole.MarkupLine("Rescans for save games and updates the internal database.");
                break;

            case "sync" or "s":
                AnsiConsole.MarkupLine("[cyan]sync[/] - Sync saves between platforms");
                AnsiConsole.MarkupLine("Copy save files between Steam and Ubisoft versions of the same game.");
                break;

            case "clear" or "cls":
                AnsiConsole.MarkupLine("[cyan]clear[/] - Clear the console screen");
                break;

            case "exit" or "quit" or "q":
                AnsiConsole.MarkupLine("[cyan]exit[/] - Exit the application");
                break;

            default:
                AnsiConsole.MarkupLine($"[red]Unknown command:[/] {command}");
                break;
        }
    }

    private async Task ListSaves()
    {
        await AnsiConsole.Status()
            .StartAsync("Loading save games...", async ctx =>
            {
                ctx.Spinner(Spinner.Known.Dots);
                ctx.SpinnerStyle = new Style(Color.Green);
                await Task.Delay(500); // Small delay for visual feedback
            });

        await ubiManager.ListSaveGamesAsync();
    }

    private async Task RenameSave(string[] args)
    {
        if (args.Length >= 3)
        {
            // Direct rename with arguments
            var gameId = args[0];
            var fileName = args[1];
            var newDisplayName = string.Join(" ", args[2..]);
            await ubiManager.RenameSaveFilesAsync(gameId, fileName, newDisplayName);
        }
        else
        {
            // Interactive rename
            await ubiManager.RenameSaveFilesAsync("", "", "");
        }
    }

    private async Task RefreshSaves()
    {
        AnsiConsole.MarkupLine("[cyan]Refreshing save games...[/]");
        
        await AnsiConsole.Status()
            .StartAsync("Scanning for save files...", async ctx =>
            {
                ctx.Spinner(Spinner.Known.Dots);
                ctx.SpinnerStyle = new Style(Color.Green);
                await ubiManager.InitializeSaveDetection();
            });
        
        AnsiConsole.MarkupLine("[green]Save games refreshed successfully![/]");
    }

    private async Task SyncSaves()
    {
        await ubiManager.SyncBetweenPlatformsAsync();
    }

    private void ClearScreen()
    {
        Console.Clear();
        AnsiConsole.Write(new FigletText("SaveManager").Centered().Color(Color.Cyan1));
        AnsiConsole.Write(Align.Center(new Markup("All clear!\nType [bold yellow]'help'[/] to see available commands!")));
    }

    private void ExitApp()
    {
        AnsiConsole.MarkupLine("[cyan]Thanks for using SaveManager![/]");
        Environment.Exit(0);
    }

    private void ShowUnknownCommand(string command)
    {
        AnsiConsole.MarkupLine($"[red]Unknown command:[/] '{command}'");
        AnsiConsole.MarkupLine("Type [yellow]'help'[/] to see available commands.");
    }
}