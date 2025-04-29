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
    private readonly Logger _logger;
    private readonly Dictionary<string, ICommand> _commands;
    private readonly bool _isRunning = true;
    
    public CommandProcessor(UbiManager ubiManager, ConfigManager configManager, Globals globals, Utilities utilities, Logger logger)
    {
        _ubiManager = ubiManager;
        _configManager = configManager;
        _globals = globals;
        _utilities = utilities;
        _logger = logger;
        _commands = new Dictionary<string, ICommand>();
        
        RegisterCommands();
    }
    
    private void RegisterCommands()
    {
        RegisterCommand(new HelpCommand(_commands));
        RegisterCommand(new ListCommand(_ubiManager, _utilities, _globals));
        RegisterCommand(new ExitCommand());
        RegisterCommand(new ClearCommand());
        // RegisterCommand(new BackupCommand(_ubiManager, _globals, _utilities, _configManager));
        RegisterCommand(new RefreshCommand(_ubiManager));
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
            try
            {
                AnsiConsole.Markup("[green]> [/]");
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
                    try
                    {
                        await command.ExecuteAsync(args);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error executing command: {ex.Message}");
                        
                        try
                        {
                            _logger.Error($"Command execution error: {ex.Message}", 2);
                        }
                        catch {  }
                    }
                }
                else
                {
                    Console.WriteLine("Unknown command. Type 'help' to see available commands.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error in command processor: {ex.Message}");
            }
        }
    }
}