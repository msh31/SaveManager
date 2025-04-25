namespace SaveManager.Commands;

abstract class CommandBase : ICommand
{
    public string Name { get; }
    public string Description { get; }
    public string Usage { get; }
    
    public CommandBase(string name, string description, string usage)
    {
        Name = name;
        Description = description;
        Usage = usage;
    }
    
    public abstract Task ExecuteAsync(string[] args);
}