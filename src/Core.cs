//ReSharper disable InconsistentNaming

using System.Diagnostics;
using SaveManager.Managers;

class Core
{
    private readonly TerminalUI _terminalUI;
    private readonly Utilities _utilities;
    private readonly ConfigManager _configManager;
    private readonly UbiManager _ubiManager;
    private readonly Logger _logger;
    private readonly CommandProcessor _commandProcessor;
    private readonly Globals _globals;
    
    public Core()
    {
        _terminalUI = new TerminalUI();
        _configManager = new ConfigManager();
        _globals = new Globals(_configManager);
        _logger = new Logger(_globals.LogFilePath, _terminalUI);
        _utilities = new Utilities(_terminalUI);
        _ubiManager = new UbiManager(_terminalUI, _configManager, _utilities, _globals);
        _commandProcessor = new CommandProcessor(_terminalUI, _ubiManager);
    }
    
    public void Initialize()
    {
        InitializeAsync().GetAwaiter().GetResult();
    }
    
    public async Task InitializeAsync()
    {
        try
        {
            if (_configManager.Data.FirstRun)
            {
                _terminalUI.WriteFormattedTextByType("First-time initialization in progress...", "inf", true, false);
                
                try
                {
                    await _ubiManager.InitializeSaveDetection();
                    //awaait _ubiManager.InitiaalizeSaveDetection();
                }
                catch (Exception ex)
                {
                    _terminalUI.WriteFormattedTextByType($"Error during initialization: {ex.Message}", "err", true, false);
                    _logger.Error($"Initialization error: {ex.Message}\n{ex.StackTrace}");
                }
                
                _configManager.Data.FirstRun = false;
                _configManager.Save();
                _terminalUI.WriteFormattedTextByType("Initialization complete!", "suc", true, false);
            }
            
            Console.Clear();
            _terminalUI.HighlightWordInText($"Welcome to SaveManager, {Environment.UserName}", ConsoleColor.Red, $"{Environment.UserName}", true, true);
            _terminalUI.HighlightWordInText("Type 'help' to see available commands!\n", ConsoleColor.Yellow, "'help'", true, true);
            _commandProcessor.Start();
        }
        catch (Exception ex)
        {
            _terminalUI.WriteFormattedTextByType($"Critical error: {ex.Message}", "err", true, false);
            _logger.Fatal($"Critical error: {ex.Message}\n{ex.StackTrace}");
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}