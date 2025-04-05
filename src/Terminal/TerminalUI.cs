//ReSharper disable InconsistentNaming
using System.Text;

class TerminalUI
{
    public TerminalUI()
    {
        Console.OutputEncoding = Encoding.UTF8;
    }
    
    public void WriteTextWithColor(string text, ConsoleColor color, bool newLine, bool centered)
    {
        if (centered) SetCenterCursorPosition(text.Length);
        
        Console.ForegroundColor = color;
        if (newLine)
            Console.WriteLine(text);
        else
            Console.Write(text);
        Console.ForegroundColor = ConsoleColor.Gray;
    }
    
    public void WriteFormattedTextByType(string text, string type, bool newLine, bool centered)
    {
        if (centered) SetCenterCursorPosition(text.Length);
        
        switch (type)
        {
            case "err":
                SetTypeTextWithColor(text, "err", ConsoleColor.Red, newLine);
                break;
            case "suc":
                SetTypeTextWithColor(text, "suc", ConsoleColor.Green, newLine);
                break;
            case "warn":
                SetTypeTextWithColor(text, "warn", ConsoleColor.Yellow, newLine);
                break;
            case "inf":
                SetTypeTextWithColor(text, "inf", ConsoleColor.Cyan, newLine);
                break;
            case "dbg":
                SetTypeTextWithColor(text, "dbg", ConsoleColor.Magenta, newLine);
                break;
            case "dbg-err":
                SetTypeTextWithColor(text, "dbg-err", ConsoleColor.Magenta, newLine);
                break;
            case "dbg-suc":
                SetTypeTextWithColor(text, "dbg-suc", ConsoleColor.Magenta, newLine);
                break;
            case "dbg-inf":
                SetTypeTextWithColor(text, "dbg-inf", ConsoleColor.Magenta, newLine);
                break;
        }
    }
    
// helpers    
    private void SetCenterCursorPosition(int textLength)
    {
        try
        {
            var left = Math.Max(0, (Console.WindowWidth - textLength) / 2);
            Console.SetCursorPosition(left, Console.CursorTop);
        }
        catch (Exception ex)
        {
            WriteTextWithColor(ex.Message, ConsoleColor.Red, true, false);
        }
    }
    
    private void SetTypeTextWithColor(string text, string type, ConsoleColor color, bool newLine)
    {
        Console.ForegroundColor = color;
        Console.Write($"[{type}] ");
        Console.ForegroundColor = ConsoleColor.Gray;
        if (newLine)
            Console.WriteLine(text);
        else
            Console.Write(text);
    }
}