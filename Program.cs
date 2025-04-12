//ReSharper disable InconsistentNaming

Console.Title = "SaveManager";
Console.ForegroundColor = ConsoleColor.Gray;

try
{
    var core = new Core();
    await core.InitializeAsync();
}
catch (Exception ex)
{
    Console.ForegroundColor = ConsoleColor.Red;
    Console.WriteLine($"Fatal error in main program: {ex.Message}");
    Console.WriteLine(ex.StackTrace);
    Console.ForegroundColor = ConsoleColor.Gray;
    Console.WriteLine("\nPress any key to exit...");
    Console.ReadKey();
}