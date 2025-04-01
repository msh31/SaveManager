//ReSharper disable InconsistentNaming

using System.Diagnostics;

class Core
{
    private readonly TerminalUI _terminalUI;
    private readonly Logger _logger;

    public Core()
    {
        _terminalUI = new TerminalUI();
        _logger = new Logger(Path.Combine(AppContext.BaseDirectory, "logs", "oops.log"), _terminalUI);
    }
    
    public void Initialize()
    {
        _terminalUI.WriteTextWithColor($"Welcome to SaveManager, {Environment.UserName}", ConsoleColor.Red, true, true);
        
        // logger tests
        try {
            _logger.Info("Application started");
        }
        catch (Exception ex) {
            _logger.Error($"Unhandled exception: {ex.Message}");
        }
        
        Console.ReadKey();
    }
}
