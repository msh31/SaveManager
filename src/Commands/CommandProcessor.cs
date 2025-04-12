//ReSharper disable InconsistentNaming

using System.Diagnostics;
using SaveManager.Commands;
using SaveManager.Managers;

class CommandProcessor
{
    private readonly TerminalUI _terminalUI;
    private readonly UbiManager _ubiManager;
    private readonly ConfigManager _configManager;
    private readonly Globals _globals;
    private readonly Utilities _utilities;
    private readonly Dictionary<string, Command> commandDictionary;
    private readonly bool isRunning;
    
    public CommandProcessor(TerminalUI terminalUI, UbiManager ubiManager, ConfigManager configManager, Globals globals, Utilities utilities)
    {
        _terminalUI = terminalUI;
        _ubiManager = ubiManager;
        _configManager = configManager;
        _globals = globals;
        _utilities = utilities;
        commandDictionary = new Dictionary<string, Command>();
        isRunning = true;
        
        RegisterCommands();
    }
    
    private void RegisterCommands()
    {
        RegisterCommand("help", "Displays available commands", "[command]");
        RegisterCommand("backup", "Creates a new backup", "<game_id> <save_index>");
        RegisterCommand("restore", "Restores from a backup", "<backup_id>");
        RegisterCommand("list", "Shows available saves", "[ubisoft|u|rockstar|r]");
        RegisterCommand("rename", "Set a display name for a save file");
        RegisterCommand("export", "Exports a save file");
        RegisterCommand("import", "Imports a save file");
        RegisterCommand("sync", "Synchronizes saves across platforms");
        RegisterCommand("clear", "Clears the console screen");
        RegisterCommand("reset", "Resets the configuration");
        RegisterCommand("exit", "Exits SaveManager");
        
        //RegisterCommand("test", "Exits SaveManager");
    }
    
    private void RegisterCommand(string name, string description, string usage = "")
    {
        commandDictionary[name.ToLower()] = new Command(name, description, usage);  
    }
    
    private async Task ProcessCommandAsync(Command command)
    {
        if (command.IsUnknown())
        {
            _terminalUI.WriteFormattedTextByType("I don't know that command. Type 'help' to see available commands.", "warn", true, false);
            return;
        }
        
        switch (command.CommandWord)
        {
            case "help":
                if (command.HasArguments()) { DisplayHelpForCommand(command.GetArgument(0)); }
                else { DisplayHelp(); }
                break;
            case "exit":
                Environment.Exit(0);
                break;
            case "clear":
                Console.Clear();
                break;
            case "reset":
                break;
            case "list":
                if (command.HasArguments())
                {
                    var platform = command.GetArgument(0).ToLower();

                    if (platform is "ubisoft" or "u")
                    {
                        await _ubiManager.ListSaveGamesAsync();
                    }
                    else if (platform is "rockstar" or "r")
                    {
                        //_rockstarManager.ListSaves();
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
                break;
            case "rename":
                if (command.HasArguments())
                {
                    var platform = command.GetArgument(0).ToLower();
        
                    if (platform is "ubisoft" or "u")
                    {
                        var gameId = command.GetArgument(1);
                        if (gameId != null)
                        {
                            await _ubiManager.RenameSaveFilesAsync(gameId);
                        }
                        else
                        {
                            _terminalUI.WriteFormattedTextByType("Please specify a game ID. Available games:", "inf", true, false);
                
                            foreach (var gameID in _configManager.Data.DetectedUbiGames)
                            {
                                var gameName = _utilities.TranslateUbisoftGameId(Path.Combine(_globals.UbisoftRootFolder, _configManager.Data.DetectedUbiAccount, gameID)).Result;
                                _terminalUI.WriteTextWithColor($"  {gameName} ({gameID})", ConsoleColor.Cyan, true, false);
                            }
                
                            _terminalUI.WriteFormattedTextByType("Use 'rename ubisoft <game_id>' to rename a save for a specific game.", "inf", true, false);
                        }
                    }
                    else if (platform is "rockstar" or "r")
                    {
                        _terminalUI.WriteFormattedTextByType("Rockstar rename feature not implemented yet.", "inf", true, false);
                    }
                    else
                    {
                        _terminalUI.WriteFormattedTextByType($"Unknown platform: {platform}. Use 'ubisoft' or 'rockstar'", "err", true, false);
                    }
                }
                else
                {
                    _terminalUI.WriteFormattedTextByType("Please specify platform: 'rename ubisoft <game_id>' or 'rename rockstar <game_id>'", "inf", true, false);
                }
                break;
            case "backup":
            case "restore":
            case "export":
            case "import":
            case "sync":
                _terminalUI.WriteFormattedTextByType($"The '{command.CommandWord}' command is not implemented yet.", "inf", true, false);
                break;
                
            default:
                _terminalUI.WriteFormattedTextByType("I don't know how to do that.", "err", true, false);
                break;
        }
    }
    
    // start the command processing loop
    public async Task StartAsync()
    {
        while (isRunning)
        {
            var command = GetCommand();
            await ProcessCommandAsync(command);
        }
    }
    
    private Command GetCommand()
    {
        _terminalUI.WriteTextWithColor("> ", ConsoleColor.Green, false, false);
        
        var input = Console.ReadLine();

        if (string.IsNullOrEmpty(input))
        {
            return new Command(null, "Empty input");
        }

        string[] words = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);
        string commandWord = words[0].ToLower();
        
        if (!IsValidCommand(commandWord))
        {
            return new Command(null, $"Unknown command: {commandWord}");
        }

        // Extract arguments (all words except the first)
        string[] args;
        if (words.Length > 1)
        {
            args = words.Skip(1).ToArray();
        }
        else
        {
            args = Array.Empty<string>();
        }
        
        var cmd = commandDictionary[commandWord];
        return new Command(commandWord, cmd.Description, cmd.Usage, args);
    }
    
    private bool IsValidCommand(string commandWord)
    {
        return commandDictionary.ContainsKey(commandWord);
    }
    
// Commands    
    private void DisplayHelp()
    {
        _terminalUI.WriteFormattedTextByType("Available commands:", "inf", true, false);
    
        foreach (var command in commandDictionary.Values)
        {
            _terminalUI.WriteTextWithColor($"  {command.CommandWord.PadRight(8)} - {command.Description}", ConsoleColor.White, true, false);
        }
    
        Console.WriteLine(string.Empty);
        _terminalUI.WriteFormattedTextByType("Type 'help <command>' for usage information.", "inf", true, false);
    }
    
    private void DisplayHelpForCommand(string commandName)
    {
        if (string.IsNullOrEmpty(commandName) || !commandDictionary.ContainsKey(commandName.ToLower()))
        {
            _terminalUI.WriteFormattedTextByType($"Unknown command: {commandName}", "err", true, false);
            return;
        }
    
        var command = commandDictionary[commandName.ToLower()];
        _terminalUI.WriteFormattedTextByType($"Command: {command.CommandWord}", "inf", true, false);
        _terminalUI.WriteTextWithColor($"  Description: {command.Description}", ConsoleColor.White, true, false);
    
        if (!string.IsNullOrEmpty(command.Usage))
        {
            _terminalUI.WriteTextWithColor($"  Usage: {command.CommandWord} {command.Usage}", ConsoleColor.White, true, false);
        }
    }
}