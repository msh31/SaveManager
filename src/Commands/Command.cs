namespace SaveManager.Commands;

public class Command
{
    public string CommandWord { get; }
    public string Description { get; }
    private string[] Arguments { get; }
    public string Usage { get; }
    
    public Command(string commandWord, string description, string usage = "", string[] args = null)
    {
        CommandWord = commandWord;
        Description = description;
        Usage = usage;
        
        if (args == null) { Arguments = new string[0]; }
        else { Arguments = args; }
    }
    
    public bool IsUnknown()
    {
        return CommandWord == null;
    }
    
// soon™️
    public bool HasArguments()
    {
        return Arguments.Length > 0;
    }
    
    public string GetArgument(int index)
    {
        if (index < Arguments.Length)
        {
            return Arguments[index];
        }
        else
        {
            return null;
        }
    }
}