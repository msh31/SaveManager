//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes
using SaveManager.Commands;
using Spectre.Console;

class RefreshCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    
    public RefreshCommand(UbiManager ubiManager) : base("refresh", "Manually refresh the savegames list", "[ubisoft|u]")
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
                AnsiConsole.MarkupLine("[cyan][[inf]][/] Refreshing Ubisoft saves...");
                await _ubiManager.InitializeSaveDetection();
                AnsiConsole.MarkupLine("[green][[suc]][/] Ubisoft saves refreshed successfully.");
            }
            else
            {
                AnsiConsole.MarkupLine($"[red][[err]][/] Unknown platform: {platform}. Use 'ubisoft'");
            }
        }
        else
        {
            AnsiConsole.MarkupLine("[cyan][[inf]][/] Please specify platform: 'refresh ubisoft'");
        }
    }
}