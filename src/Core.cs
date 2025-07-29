//ReSharper disable InconsistentNaming

using System.Diagnostics;
using Spectre.Console;
using SaveManager.Utilities;
using SaveManager.Managers;
using SaveManager.Services;

class Core : IDisposable
{
    private readonly Utilities utilities;
    private readonly ConfigManager configManager;
    private readonly Logger logger;
    private readonly SaveManager.Utilities.SaveManager saveManager;
    private readonly CommandLoop commandLoop;
    private readonly Globals globals;
    private readonly GameArtwork artworkService;

    public Core()
    {
        // var apiKey = configManager.Data.SteamGridDbApiKey ?? string.Empty;
        
        globals = new Globals();
        configManager = new ConfigManager(globals.ConfigFilePath);
        globals.UpdateConfig(configManager);
        logger = new Logger(globals.LogFilePath);
        utilities = new Utilities(logger);
        saveManager = new SaveManager.Utilities.SaveManager(globals, utilities, configManager, logger);
        // artworkService = new GameArtwork(apiKey);
        artworkService = new GameArtwork("01b8e27efe8f5599af5b165bf41aad4f");
        
        commandLoop = new CommandLoop(saveManager, globals, utilities, artworkService);
    }

    public async Task InitializeAsync()
    {
        try {
            logger.Info("Initializing Application");
        
            await saveManager.InitializeAsync();

            if (!Debugger.IsAttached) {
                Console.Clear();
            }
            ShowWelcomeMessage();
            
            await commandLoop.StartAsync();
        }
        catch (Exception ex) {
            logger.Fatal($"Error during initialization: {ex.Message}\n{ex.StackTrace}\n\nPress any key to exit", 1, true);
            Console.ReadKey();
        }
    }

    private static void ShowWelcomeMessage()
    {
        AnsiConsole.Write(new FigletText("SaveManager").Centered().Color(Color.Cyan1));
        AnsiConsole.Write(Align.Center(new Markup($"Welcome, [red]{Environment.UserName}[/] | Version: [bold red]TEST[/]\nType [bold yellow]'help'[/] to see available commands!")));
    }
    
    
    public void Dispose() {
        artworkService?.Dispose();
    }
}