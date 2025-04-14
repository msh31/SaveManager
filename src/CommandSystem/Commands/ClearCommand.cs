//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes

using SaveManager.Commands;

class ClearCommand : ICommand
{
    public string Name => "clear";
    public string Description => "Clears the console screen";
    public string Usage => "";
    
    public Task ExecuteAsync(string[] args)
    {
        Console.Clear();
        return Task.CompletedTask;
    }
}