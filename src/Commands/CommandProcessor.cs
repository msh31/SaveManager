//ReSharper disable InconsistentNaming

using SaveManager.Commands;

class CommandProcessor
{
    private readonly TerminalUI _terminalUI;
    private readonly Logger _logger;
    private readonly Dictionary<string, Command> commandDictionary;
    private readonly bool isRunning;
    
    public CommandProcessor(TerminalUI terminalUI, Logger logger)
    {
        _terminalUI = terminalUI;
        _logger = logger;
        commandDictionary = new Dictionary<string, Command>();
        isRunning = true;
        
        RegisterCommands();
    }
    
    private void RegisterCommands()
    {
        RegisterCommand("help", "Displays available commands");
        RegisterCommand("backup", "Creates a new backup");
        RegisterCommand("restore", "Restores from a backup");
        RegisterCommand("list", "Shows available saves");
        RegisterCommand("rename", "Set a display name for a save file");
        RegisterCommand("export", "Exports a save file");
        RegisterCommand("import", "Imports a save file");
        RegisterCommand("sync", "Synchronizes saves across platforms");
        RegisterCommand("clear", "Clears the console screen");
        RegisterCommand("reset", "Resets the configuration");
        RegisterCommand("exit", "Exits SaveManager");
        
        //RegisterCommand("test", "Exits SaveManager");
    }
    
    private void RegisterCommand(string name, string description)
    {
        commandDictionary[name.ToLower()] = new Command(name, description);
    }
    
    private void ProcessCommand(Command command)
    {
        if (command.IsUnknown())
        {
            _terminalUI.WriteFormattedTextByType("I don't know that command. Type 'help' to see available commands.", "warn", true, false);
            return;
        }
        
        switch (command.CommandWord)
        {
            case "help":
                DisplayHelp();
                break;
            case "exit":
                Environment.Exit(0);
                break;
            case "clear":
                Console.Clear();
                break;
            case "reset":
            case "list":
            case "rename":
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
    public void Start()
    {
        while (isRunning)
        {
            var command = GetCommand();
            ProcessCommand(command);
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
        
        return new Command(commandWord, commandDictionary[commandWord].Description, args);
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
    }
}