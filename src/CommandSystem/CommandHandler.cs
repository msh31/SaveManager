using System.Diagnostics;
using Spectre.Console;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using SaveManager.Models;
using SaveManager.Services;

namespace SaveManager.CommandSystem;

internal class CommandHandler
{
    private readonly SaveManager.Utilities.SaveManager saveManager;
    private readonly Globals globals;
    private readonly Utilities.Utilities utilities;
    private readonly GameArtwork artwork;

    public CommandHandler(SaveManager.Utilities.SaveManager saveManager, Globals globals, Utilities.Utilities utilities, GameArtwork artwork)
    {
        this.saveManager = saveManager;
        this.globals = globals;
        this.utilities = utilities;
        this.artwork = artwork;
    }

    public async Task ExecuteAsync(string input)
    {
        if (string.IsNullOrWhiteSpace(input)) 
            return;

        var parts = input.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);
        var command = parts[0].ToLower();
        var args = parts.Skip(1).ToArray();

        try {
            switch (command) {
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
                case "serve":
                    // Check for background flag - this is about running the server in background
                    bool runInBackground = args.Contains("--background") || args.Contains("-b");
                    await StartWebServer(runInBackground);
                    break;
                default:
                    ShowUnknownCommand(command);
                    break;
            }
        } catch (Exception ex) {
            AnsiConsole.MarkupLine($"[red]Error executing command:[/] {ex.Message}");
        }
    }

    private async Task StartWebServer(bool runInBackground) {
        if (runInBackground) {
            StartBackgroundServer();
            return;
        }

        // Run server in foreground (blocking)
        var builder = WebApplication.CreateBuilder();
        builder.Services.AddSingleton(saveManager);
        builder.Logging.ClearProviders();

        var app = builder.Build();
        app.UseStaticFiles();
        app.MapGet("/api/saves", HandleSavesRequest);
        app.MapFallbackToFile("index.html");

        AnsiConsole.MarkupLine("[green]Starting SaveManager web interface...[/]");
        AnsiConsole.MarkupLine("[cyan]Open your browser to: http://localhost:5000[/]");
        AnsiConsole.MarkupLine("[yellow]Press Ctrl+C to stop and return to CLI[/]");

        await app.RunAsync("http://localhost:5000");
    }

    private async Task<IResult> HandleSavesRequest() {
        if (!File.Exists(globals.UbiSaveInfoFilePath)) {
            return Results.Json(new { error = "No save data found" });
        }

        try {
            var json = await File.ReadAllTextAsync(globals.UbiSaveInfoFilePath);
            var rawData = System.Text.Json.JsonSerializer.Deserialize(json, JsonContext.Default.DictionaryStringListSaveFileInfo);

            if (rawData == null) {
                return Results.Json(new { error = "Invalid save data" });
            }

            var enhancedData = new Dictionary<string, object>();

            foreach (var entry in rawData) {
                var key = entry.Key;
                var saves = entry.Value;
                var parts = key.Split('_', 2);
                
                if (parts.Length != 2) {
                    continue;
                }

                var accountId = parts[0];
                var gameId = parts[1];
                
                var gameName = await utilities.TranslateUbisoftGameId(Path.Combine(globals.UbisoftRootFolder, accountId, gameId));
                
                string artworkUrl = null;
                try {
                    artworkUrl = await artwork.GetGameArtworkAsync(gameName);
                } catch (Exception ex) {
                    Console.WriteLine($"Failed to get artwork for '{gameName}': {ex.Message}");
                }

                enhancedData[key] = new {
                    GameName = gameName,
                    AccountId = accountId,
                    ArtworkUrl = artworkUrl,
                    Saves = saves
                };
            }

            return Results.Json(enhancedData);
        } catch (Exception ex) {
            return Results.Json(new { error = $"Error processing save data: {ex.Message}" });
        }
    }

    private void StartBackgroundServer()
    {
        var process = new ProcessStartInfo
        {
            FileName = Environment.ProcessPath,
            Arguments = "serve",
            UseShellExecute = true,
            CreateNoWindow = false,
            WindowStyle = ProcessWindowStyle.Minimized
        };

        Process.Start(process);
        AnsiConsole.MarkupLine("[green]Web interface started in background at http://localhost:5000[/]");
        AnsiConsole.MarkupLine("[cyan]Type 'help' to continue using CLI commands[/]");
    }
    
    private async Task ListSaves()
    {
        await AnsiConsole.Status()
            .StartAsync("Loading save games...", async ctx =>
            {
                ctx.Spinner(Spinner.Known.Dots);
                ctx.SpinnerStyle = new Style(Color.Green);
                await saveManager.ListSaves("Ubisoft");
            });
    }

    private async Task RenameSave(string[] args) {
        if (args.Length >= 3) {
            await saveManager.RenameSave("Ubisoft", args[0], args[1], string.Join(" ", args[2..]));
        } else {
            AnsiConsole.MarkupLine("[yellow]Usage: rename <gameId> <fileName> <newName>[/]");
        }
    }

    private async Task RefreshSaves()
    {
        await AnsiConsole.Status().StartAsync("Scanning for save files...", async ctx => {
                ctx.Spinner(Spinner.Known.Dots);
                ctx.SpinnerStyle = new Style(Color.Green);
                await saveManager.InitializeAsync();
            });
    }

    private async Task SyncSaves()
    {
        AnsiConsole.MarkupLine("[yellow]Sync functionality coming soon![/]");
    }

    private void ShowHelp(string[] args) 
    {
        if (args.Length > 0) {
            ShowSpecificHelp(args[0]);
            return;
        }

        AnsiConsole.MarkupLine("[cyan]Available commands:[/]");
        AnsiConsole.MarkupLine("  [yellow]list[/] (l)      - List all save games");
        AnsiConsole.MarkupLine("  [yellow]rename[/] (r)    - Rename a save file");
        AnsiConsole.MarkupLine("  [yellow]refresh[/]       - Refresh save games list");
        AnsiConsole.MarkupLine("  [yellow]serve[/]         - Start web interface");
        AnsiConsole.MarkupLine("  [yellow]sync[/] (s)      - Sync saves between platforms");
        AnsiConsole.MarkupLine("  [yellow]clear[/] (cls)   - Clear the console");
        AnsiConsole.MarkupLine("  [yellow]exit[/] (q)      - Exit the application");
        AnsiConsole.MarkupLine("  [yellow]help[/] (h)      - Show this help");
        AnsiConsole.MarkupLine("\n[cyan]Tip:[/] Type [yellow]'help <command>'[/] for detailed usage.");
    }

    private void ShowSpecificHelp(string command) 
    {
        switch (command.ToLower()) {
            case "list" or "l":
                AnsiConsole.MarkupLine("[cyan]list[/] - List all save games");
                AnsiConsole.MarkupLine("Shows all detected save games organized by game and platform.");
                break;
            case "rename" or "r":
                AnsiConsole.MarkupLine("[cyan]rename[/] - Rename a save file");
                AnsiConsole.MarkupLine("Usage: [yellow]rename <gameId> <fileName> <newName>[/]");
                break;
            case "refresh":
                AnsiConsole.MarkupLine("[cyan]refresh[/] - Refresh save games list");
                AnsiConsole.MarkupLine("Rescans for new save games and updates the database.");
                break;
            case "serve":
                AnsiConsole.MarkupLine("[cyan]serve[/] - Start web interface");
                AnsiConsole.MarkupLine("Starts the web interface at http://localhost:5000");
                AnsiConsole.MarkupLine("Use [yellow]serve --background[/] to run in background.");
                break;
            case "sync" or "s":
                AnsiConsole.MarkupLine("[cyan]sync[/] - Sync saves between platforms");
                AnsiConsole.MarkupLine("Copy save files between Steam and Ubisoft versions.");
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