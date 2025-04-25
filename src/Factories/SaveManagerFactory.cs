//ReSharper disable InconsistentNaming
using SaveManager.Interfaces;
using SaveManager.Managers;

class SaveManagerFactory
{
    private readonly ConfigManager _configManager;
    private readonly Globals _globals;
    private readonly Utilities _utilities;
    private readonly Logger _logger;
    
    public SaveManagerFactory(ConfigManager configManager, Globals globals, Utilities utilities, Logger logger)
    {
        _configManager = configManager;
        _globals = globals;
        _utilities = utilities;
        _logger = logger;
    }
    
    public ISaveManager CreateManager(string platform)
    {
        return platform.ToLower() switch
        {
            "ubisoft" => new UbiManager(_configManager, _utilities, _globals, _logger),
            //"rockstar" => new RockstarManager(_terminalUI, _globals),
            _ => throw new ArgumentException($"Unsupported platform: {platform}")
        };
    }
}