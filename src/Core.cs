//ReSharper disable InconsistentNaming

using System.Diagnostics;
using SaveManager.Managers;
using Spectre.Console;

class Core
{
    private readonly Utilities _utilities;
    private readonly ConfigManager _configManager;
    private readonly UbiManager _ubiManager;
    private readonly Logger _logger;
    private readonly CommandProcessor _commandProcessor;
    private readonly Globals _globals;
    
    private readonly SaveManagerFactory _saveManagerFactory;
    
    public Core()
    {
        _globals = new Globals();
        _configManager = new ConfigManager(_globals.ConfigFilePath);
        _globals.UpdateConfig(_configManager);
        _logger = new Logger(_globals.LogFilePath);
        _utilities = new Utilities(_logger);
        _saveManagerFactory = new SaveManagerFactory(_configManager, _globals, _utilities, _logger);
        _ubiManager = (UbiManager)_saveManagerFactory.CreateManager("ubisoft");
        _commandProcessor = new CommandProcessor(_ubiManager, _configManager, _globals, _utilities);
    }
    
    public async Task InitializeAsync()
    {
        try
        {
            if (_configManager.Data.FirstRun)
            {
                _logger.Info("First-time initialization in progress...", 0, true);
                
                try
                {
                    await _ubiManager.InitializeSaveDetection();
                }
                catch (Exception ex)
                {
                    _logger.Error($"[red][[err]][/] Initialization error: {ex.Message}\n{ex.StackTrace}", 0, true);
                }
                
                _configManager.Data.FirstRun = false;
                _configManager.Save();
                _logger.Info("Initialization complete!", 0, true);
            }
            
            Console.Clear();
            AnsiConsole.Write(new FigletText("SaveManager").Centered().Color(Color.Cyan1));
            AnsiConsole.Write(Align.Center(new Markup($"Welcome, [red]{Environment.UserName}[/]\nType [bold yellow]'help'[/] to see available commands!")));
            
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