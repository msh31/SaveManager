//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes

using SaveManager.Commands;

class ExitCommand : ICommand
{
    public string Name => "exit";
    public string Description => "Closes SaveManager";
    public string Usage => "";
    
    public Task ExecuteAsync(string[] args)
    {
        Environment.Exit(0);
        return Task.CompletedTask;
    }
}