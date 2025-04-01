//ReSharper disable InconsistentNaming

class Core
{
    private TerminalUI terminalUI;

    public Core()
    {
        terminalUI = new TerminalUI();
        
    }
    
    public void Initialize()
    {
        terminalUI.WriteTextWithColor($"Welcome to SaveManager, {Environment.UserName}", ConsoleColor.Red, true, true);
        Console.ReadKey();
    }
}
