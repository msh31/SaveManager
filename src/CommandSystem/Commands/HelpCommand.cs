//ReSharper disable InconsistentNaming
using SaveManager.Commands;
using SaveManager.Managers;
using Spectre.Console;

class HelpCommand : CommandBase
{
    private readonly Dictionary<string, ICommand> _commands;
    
    public HelpCommand(Dictionary<string, ICommand> commands) : base("help", "Displays available commands", "[command]")
    {
        _commands = commands;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        if (args.Length > 0)
        {
            var commandName = args[0];

            if (string.IsNullOrEmpty(commandName) || !_commands.ContainsKey(commandName.ToLower()))
            {
                AnsiConsole.Write(new Markup($"[orange][[warn]][/] Unknown command: {commandName}\n"));
                return;
            }
    
            var command = _commands[commandName.ToLower()];
            AnsiConsole.Write(new Markup($"[cyan][[inf]][/] Command: {command.Name}\n"));
            AnsiConsole.Write(new Markup($"[white]  Description: {command.Description}[/]\n"));
    
            if (!string.IsNullOrEmpty(command.Usage))
            {
                AnsiConsole.Write(new Markup($"[white]    Usage: {command.Name} {command.Usage}[/]\n"));
            }
        }
        else
        {
            AnsiConsole.Write(new Markup("[cyan][[inf]][/] Available commands:\n"));
            
            foreach (var command in _commands.Values)
            {
                AnsiConsole.Write(new Markup($"[white] {command.Name.PadRight(8)} - {command.Description}[/]\n"));
            }
            
            AnsiConsole.Write(new Markup("\n[cyan][[inf]][/] Type 'help <command>' for usage information.\n"));
        }
    }
}