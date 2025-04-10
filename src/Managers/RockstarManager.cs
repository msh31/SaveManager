//ReSharper disable InconsistentNaming

class RockstarManager : BaseManager
{
    private readonly TerminalUI _terminalUI;
    private readonly Globals _globals;
    
    public RockstarManager(TerminalUI terminalUI, Globals globals) : base(terminalUI, globals, "rockstargames")
    {
        _terminalUI = terminalUI;
        _globals = globals;
    }
    
    //TODO
}