//ReSharper disable InconsistentNaming
using SaveManager.Commands;
using SaveManager.Managers;

class ListCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    private readonly TerminalUI _terminalUI;
    
    public ListCommand(TerminalUI terminalUI, UbiManager ubiManager) : base("list", "Shows available saves", "[ubisoft|u|rockstar|r]", terminalUI)
    {
        _ubiManager = ubiManager;
        _terminalUI = terminalUI;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        if (args.Length > 0)
        {
            var platform = args[0].ToLower();
            
            if (platform is "ubisoft" or "u")
            {
                await _ubiManager.ListSaveGamesAsync();
            }
            else if (platform is "rockstar" or "r")
            {
                _terminalUI.WriteFormattedTextByType("Rockstar list not implemented yet.", "inf", true, false);
            }
            else
            {
                _terminalUI.WriteFormattedTextByType($"Unknown platform: {platform}. Use 'ubisoft' or 'rockstar'", "err", true, false);
            }
        }
        else
        {
            _terminalUI.WriteFormattedTextByType("Please specify platform: 'list ubisoft' or 'list rockstar'", "inf", true, false);
        }
    }
}