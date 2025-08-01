//ReSharper disable InconsistentNaming

using System.Diagnostics;
using SaveManager.Managers;
using SaveManager.Services;
using SaveManager.Services;
using SaveManager.Utilities;
using Spectre.Console;

class Core : IDisposable
{
    private readonly Utilities utilities;
    private readonly ConfigManager configManager;
    private readonly Logger logger;
    private readonly SaveManager.Services.SaveManager saveManager;
    private readonly CommandLoop commandLoop;
    private readonly Globals globals;
    private readonly GameArtwork artworkService;

    public Core()
    {
        globals = new Globals();
        configManager = new ConfigManager(globals.ConfigFilePath);
        globals.UpdateConfig(configManager);
        logger = new Logger(globals.LogFilePath);
        utilities = new Utilities(logger);
        saveManager = new SaveManager.Services.SaveManager(globals, utilities, configManager, logger);
        artworkService = new GameArtwork(configManager.Data.SteamGridDbApiKey);
        
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
        } catch (InvalidOperationException ex) when (ex.Message.Contains("Ubisoft savegame folder")) {
            Console.Error.WriteLine("[error] Could not find any Ubisoft savegame data on this system.");
            Console.WriteLine("This may happen if Ubisoft Connect is not installed or no games have been launched yet.");
            Console.WriteLine();
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
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
    }}