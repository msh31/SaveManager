//ReSharper disable InconsistentNaming
using SaveManager.Commands;
using SaveManager.Managers;

class HelpCommand : CommandBase
{
    private readonly TerminalUI _terminalUI;
    private readonly Dictionary<string, ICommand> _commands;
    
    public HelpCommand(TerminalUI terminalUI, Dictionary<string, ICommand> commands) : base("help", "Displays available commands", "[command]", terminalUI)
    {
        _terminalUI = terminalUI;
        _commands = commands;
    }
    
    public override async Task ExecuteAsync(string[] args)
    {
        if (args.Length > 0)
        {
            var commandName = args[0];

            if (string.IsNullOrEmpty(commandName) || !_commands.ContainsKey(commandName.ToLower()))
            {
                _terminalUI.WriteFormattedTextByType($"Unknown command: {commandName}", "err", true, false);
                return;
            }
    
            var command = _commands[commandName.ToLower()];
            _terminalUI.WriteFormattedTextByType($"Command: {command.Name}", "inf", true, false);
            _terminalUI.WriteTextWithColor($"  Description: {command.Description}", ConsoleColor.White, true, false);
    
            if (!string.IsNullOrEmpty(command.Usage))
            {
                _terminalUI.WriteTextWithColor($"  Usage: {command.Name} {command.Usage}", ConsoleColor.White, true, false);
            }
        }
        else
        {
            _terminalUI.WriteFormattedTextByType("Available commands:", "inf", true, false);
            
            foreach (var command in _commands.Values)
            {
                _terminalUI.WriteTextWithColor($"  {command.Name.PadRight(8)} - {command.Description}", ConsoleColor.White, true, false);
            }
    
            Console.WriteLine(string.Empty);
            _terminalUI.WriteFormattedTextByType("Type 'help <command>' for usage information.", "inf", true, false);
        }
    }
}