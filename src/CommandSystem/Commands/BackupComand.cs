//ReSharper disable InconsistentNaming
using SaveManager.Commands;
using SaveManager.Managers;
using Spectre.Console;

class BackupCommand : CommandBase
{
    private readonly UbiManager _ubiManager;
    
    // backup create 1081
    // backup delete 1081
    // backup update all
    public BackupCommand(UbiManager ubiManager) : base("backup", "Shows available saves", "[create|delete|update| gameID / name ]")
    {
        _ubiManager = ubiManager;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        if (args.Length > 0)
        {
            var type = args[0].ToLower();

            if (args.Length > 1)
            {
                switch (type)
                {
                    case "create":
                        //_ubiManager.BackupSaveGame(args[1], );
                        break;
                    case "delete":
                        break;
                    case "update":
                        break;
                    default:
                        AnsiConsole.Write(new Markup("[orange][[warn]][/] Invalid argument given! \n"));
                        break;
                }       
            }
        }
        else
        {
            AnsiConsole.Write(new Markup("[cyan][[inf][/] Please specify platform: 'list ubisoft' or 'list rockstar'"));
        }
    }
}