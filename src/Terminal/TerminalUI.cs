//ReSharper disable InconsistentNaming
using System.Text;

class TerminalUI
{
    public TerminalUI()
    {
        Console.OutputEncoding = Encoding.UTF8;
    }
    
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
}