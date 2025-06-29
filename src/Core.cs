//ReSharper disable InconsistentNaming

using System.Diagnostics;
using SaveManager.Managers;
using Spectre.Console;

class Core
{
    private readonly Utilities utilities;
    private readonly ConfigManager configManager;
    private readonly UbiManager ubiManager;
    private readonly Logger logger;
    private readonly CommandLoop commandLoop;
    private readonly Globals globals;

    public Core()
    {
        globals = new Globals();
        configManager = new ConfigManager(globals.ConfigFilePath);
        globals.UpdateConfig(configManager);
        logger = new Logger(globals.LogFilePath);
        utilities = new Utilities(logger);
        
        // Direct instantiation - no factory needed
        ubiManager = new UbiManager(configManager, utilities, globals, logger);
        commandLoop = new CommandLoop(ubiManager);
    }

    public async Task InitializeAsync()
    {
        try
        {
            logger.Info("Initializing Application");
            await ubiManager.InitializeSaveDetection();

            if (!Debugger.IsAttached) Console.Clear();
            ShowWelcomeMessage();

            await commandLoop.StartAsync();
        }
        catch (Exception ex)
        {
            logger.Fatal($"Error during initialization: {ex.Message}\n{ex.StackTrace}\n\nPress any key to exit", 1, true);
            Console.ReadKey();
        }
    }

    private static void ShowWelcomeMessage()
    {
        AnsiConsole.Write(new FigletText("SaveManager").Centered().Color(Color.Cyan1));
        AnsiConsole.Write(Align.Center(new Markup($"Welcome, [red]{Environment.UserName}[/]\nType [bold yellow]'help'[/] to see available commands!")));
    }
}