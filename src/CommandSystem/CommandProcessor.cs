//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes

using System.Diagnostics;
using SaveManager.Commands;
using SaveManager.Managers;
using Spectre.Console;

class CommandProcessor
{
    private readonly UbiManager _ubiManager;
    private readonly ConfigManager _configManager;
    private readonly Globals _globals;
    private readonly Utilities _utilities;
    private readonly Dictionary<string, ICommand> _commands;
    private readonly bool _isRunning = true;
    
    public CommandProcessor(UbiManager ubiManager, ConfigManager configManager, Globals globals, Utilities utilities)
    {
        _ubiManager = ubiManager;
        _configManager = configManager;
        _globals = globals;
        _utilities = utilities;
        _commands = new Dictionary<string, ICommand>();
        
        RegisterCommands();
    }
    
    private void RegisterCommands()
    {
        RegisterCommand(new HelpCommand(_commands));
        RegisterCommand(new ListCommand(_ubiManager));
        RegisterCommand(new ExitCommand());
        RegisterCommand(new ClearCommand());
        // RegisterCommand(new RenameCommand(_terminalUI, _ubiManager, _configManager, _globals, _utilities));
    }
    
    private void RegisterCommand(ICommand command)
    {
        _commands[command.Name.ToLower()] = command;
    }
    
    // start the command processing loop
    public async Task StartAsync()
    {
        while (_isRunning)
        {
            AnsiConsole.Write(new Markup("[green]> [/]"));
            var input = Console.ReadLine();
            
            if (string.IsNullOrEmpty(input))
            {
                continue;
            }
            
            string[] parts = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            string commandName = parts[0].ToLower();
            string[] args = parts.Length > 1 ? parts.Skip(1).ToArray() : Array.Empty<string>();
            
            if (_commands.TryGetValue(commandName, out var command))
            {
                await command.ExecuteAsync(args);
            }
            else
            {
                AnsiConsole.Write(new Markup("[orange][[warn]][/] I don't know that command. Type 'help' to see available commands.\n"));
            }
        }
    }
}