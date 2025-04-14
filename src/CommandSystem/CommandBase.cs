namespace SaveManager.Commands;

abstract class CommandBase : ICommand
{
    public string Name { get; }
    public string Description { get; }
    public string Usage { get; }
    private readonly TerminalUI _terminalUI;
    
    public CommandBase(string name, string description, string usage, TerminalUI terminalUI)
    {
        Name = name;
        Description = description;
        Usage = usage;
        _terminalUI = terminalUI;
    }
    
    public abstract Task ExecuteAsync(string[] args);
}