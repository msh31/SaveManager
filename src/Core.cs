//ReSharper disable InconsistentNaming

using System.Diagnostics;
using SaveManager.Managers;

class Core
{
    private readonly TerminalUI _terminalUI;
    //private readonly Logger _logger;
    private readonly CommandProcessor _commandProcessor;
    private readonly ConfigManager _config;

    public Core()
    {
        _terminalUI = new TerminalUI();
        _config = new ConfigManager();
        //_logger = new Logger(Path.Combine(AppContext.BaseDirectory, "logs", "oops.log"), _terminalUI);
        _commandProcessor = new CommandProcessor(_terminalUI);
    }
    
    public void Initialize()
    {
        Console.Clear();
        _terminalUI.HighlightWordInText($"Welcome to SaveManager, {Environment.UserName}", ConsoleColor.Red, $"{Environment.UserName}", true, true);
        _terminalUI.HighlightWordInText("Type 'help' to see available commands!\n", ConsoleColor.Yellow, "'help'", true, true);
        
        if (_config.Data.FirstRun)
        {
            // TODO: add first time initialization (directories, detections (game + accounts), etc.)
            _config.Data.FirstRun = false;
            _config.Save();
        }
        
        // start command loop
        _commandProcessor.Start();
        
        // logger tests
        // try {
        //     _logger.Info("Application started");
        //     _logger.Warning("warning message");
        // }
        // catch (Exception ex) {
        //     _logger.Error($"Unhandled exception: {ex.Message}");
        // }
        
        Console.ReadKey();
    }
}