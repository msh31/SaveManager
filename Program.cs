//ReSharper disable InconsistentNaming

using Spectre.Console;

Console.Title = "SaveManager";
Console.ForegroundColor = ConsoleColor.Gray;

try
{
    var core = new Core();
    await core.InitializeAsync();
}
catch (Exception ex)
{
    AnsiConsole.Write(new Markup($"[red]Fatal error in main program:[/] {ex.Message}\n"));
    AnsiConsole.Write(new Markup($"[red bold underline]STACKTRACE:[/] {ex.StackTrace}\n"));
    Console.WriteLine("Press any key to exit...");
    Console.ReadKey();
}