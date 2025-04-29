//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes
using SaveManager.Commands;
using Spectre.Console;

class SyncCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    
    public SyncCommand(UbiManager ubiManager) : base("sync", "sync saves across platforms (ubisoft only)", "[ubisoft|u]")
    {
        _ubiManager = ubiManager;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        if (args.Length > 0)
        {
            var platform = args[0].ToLower();
            
            if (platform is "ubisoft" or "u")
            {
                await _ubiManager.SyncBetweenPlatformsAsync();
                return;
            }
            
            AnsiConsole.MarkupLine($"[darkorange3][[warn]][/] Unknown platform: {platform}. Use 'ubisoft|u'");
        }
        else
        {
            AnsiConsole.MarkupLine($"[cyan][[inf]][/] Add a platform. Use 'ubisoft|u'");
        }
    }
}