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
    
    private readonly SaveManagerFactory _saveManagerFactory;
    
    public Core()
    {
        _terminalUI = new TerminalUI();
        _globals = new Globals();
        _configManager = new ConfigManager(_globals.ConfigFilePath);
        _globals.UpdateConfig(_configManager);
        _logger = new Logger(_globals.LogFilePath, _terminalUI);
        _utilities = new Utilities(_terminalUI);
        _saveManagerFactory = new SaveManagerFactory(_terminalUI, _configManager, _globals, _utilities);
        _ubiManager = (UbiManager)_saveManagerFactory.CreateManager("ubisoft");
        _commandProcessor = new CommandProcessor(_terminalUI, _ubiManager, _configManager, _globals, _utilities);
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
                }
                catch (Exception ex)
                {
                    _logger.Error($"Initialization error: {ex.Message}\n{ex.StackTrace}", 0, true);
                }
                
                _configManager.Data.FirstRun = false;
                _configManager.Save();
                _logger.Info("Initialization complete!");
                _terminalUI.WriteFormattedTextByType("Initialization complete!", "suc", true, false);
            }
            
            Console.Clear();
            _terminalUI.HighlightWordInText($"Welcome to SaveManager, {Environment.UserName}", ConsoleColor.Red, $"{Environment.UserName}", true, true);
            _terminalUI.HighlightWordInText("Type 'help' to see available commands!\n", ConsoleColor.Yellow, "'help'", true, true);
            
            await _commandProcessor.StartAsync();
        }
        catch (Exception ex)
        {
            _logger.Fatal($"Critical error during initialization: {ex.Message}\n{ex.StackTrace}", 0, true);
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}