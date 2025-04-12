//ReSharper disable InconsistentNaming
//namespace SaveManager.Managers;

// base class for platform managers
internal class BaseManager
{
    private readonly TerminalUI _terminalUI;
    private readonly Globals _globals;
    private readonly string _backupDirectory;
    
    public BaseManager(TerminalUI terminalUI, Globals globals, string publisherName)
    {
        _terminalUI = terminalUI;
        _globals = globals;
        
        _backupDirectory = Path.Combine(_globals.BackupsFolder, publisherName);
        Directory.CreateDirectory(_backupDirectory);
    }
    
    //currently unused (sorta) until I can think of universal methods
}