//ReSharper disable InconsistentNaming
using SaveManager.Interfaces;
using SaveManager.Managers;

class SaveManagerFactory
{
    private readonly TerminalUI _terminalUI;
    private readonly ConfigManager _configManager;
    private readonly Globals _globals;
    private readonly Utilities _utilities;
    
    public SaveManagerFactory(TerminalUI terminalUI, ConfigManager configManager, Globals globals, Utilities utilities)
    {
        _terminalUI = terminalUI;
        _configManager = configManager;
        _globals = globals;
        _utilities = utilities;
    }
    
    public ISaveManager CreateManager(string platform)
    {
        return platform.ToLower() switch
        {
            "ubisoft" => new UbiManager(_terminalUI, _configManager, _utilities, _globals),
            //"rockstar" => new RockstarManager(_terminalUI, _globals),
            _ => throw new ArgumentException($"Unsupported platform: {platform}")
        };
    }
}