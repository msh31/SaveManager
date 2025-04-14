namespace SaveManager.Commands;

public interface ICommand
{
    string Name { get; }
    string Description { get; }
    string Usage { get; }
    Task ExecuteAsync(string[] args);
}