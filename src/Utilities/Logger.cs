using System.Text;
using Spectre.Console;
using System.Text.RegularExpressions;

public enum LogLevel
{
    Debug,
    Info,
    Warning,
    Error,
    Fatal
}

class Logger
{
    private readonly string _logFilePath;
    private LogLevel _minimumLogLevel = LogLevel.Info;
    private bool _includeTimestamp = true;
    private readonly object _fileLock = new();
    
    public Logger(string logFilePath)
    {
        _logFilePath = logFilePath;
        Directory.CreateDirectory(Path.GetDirectoryName(logFilePath));
    }
    
    public void SetMinimumLogLevel(LogLevel level)
    {
        _minimumLogLevel = level;
    }
    
    public void Debug(string message, int indentLevel = 0, bool consoleOutput = false)
    {
        if (_minimumLogLevel <= LogLevel.Debug)
            WriteLogEntry(message, "DEBUG", indentLevel, "chartreuse2", consoleOutput);
    }
    
    public void Info(string message, int indentLevel = 0, bool consoleOutput = false)
    {
        if (_minimumLogLevel <= LogLevel.Info)
            WriteLogEntry(message, "INFO", indentLevel, "cyan", consoleOutput);
    }
    
    public void Warning(string message, int indentLevel = 0, bool consoleOutput = false)
    {
        if (_minimumLogLevel <= LogLevel.Warning)
            WriteLogEntry(message, "WARN", indentLevel, "yellow", consoleOutput);
    }
    
    public void Error(string message, int indentLevel = 0, bool consoleOutput = false)
    {
        if (_minimumLogLevel <= LogLevel.Error)
            WriteLogEntry(message, "ERROR", indentLevel, "red", consoleOutput);
    }
    
    public void Fatal(string message, int indentLevel = 0, bool consoleOutput = false)
    {
        if (_minimumLogLevel <= LogLevel.Fatal)
            WriteLogEntry(message, "FATAL", indentLevel, "darkred_1", consoleOutput);
    }
    
    private void WriteLogEntry(string message, string levelName, int indentLevel, string color, bool consoleOutput)
    {
        try
        {
            var cleanMessage = StripMarkup(message);
            var logEntry = FormatLogEntry(cleanMessage, levelName, indentLevel);
            
            lock (_fileLock)
            {
                using var writer = new StreamWriter(_logFilePath, true);
                writer.WriteLine(logEntry);
            }

            if (!consoleOutput) return;
            
            AnsiConsole.MarkupLine($"[{color}]{levelName}[/] {message}");
        }
        catch (IOException ex)
        {
            AnsiConsole.MarkupLine($"[red]Failed to write to log file:[/] {ex.Message}");
        }
        catch (Exception ex)
        {
            AnsiConsole.MarkupLine($"[red]Unexpected error writing to log:[/] {ex.Message}");
        }
    }
    
    private string FormatLogEntry(string message, string levelName, int indentLevel)
    {
        var sb = new StringBuilder();
        
        if (_includeTimestamp)
        {
            sb.Append($"{DateTime.Now:yyyy-MM-dd HH:mm:ss} ");
        }
        
        sb.Append(new string(' ', indentLevel * 2));
        sb.Append($"[{levelName}] {message}");
        
        return sb.ToString();
    }
    
// Helper method to strip Spectre.Console markup tags from text
    private string StripMarkup(string text)
    {
        if (string.IsNullOrEmpty(text))
        {
            return text;
        }
            
        // Remove [color]...[/] style tags
        text = Regex.Replace(text, @"\[([^\]]*)\](.*?)\[\/\]", "$2");
        
        // Remove any remaining square bracket tags
        text = Regex.Replace(text, @"\[(.*?)\]", "[$1]");
        
        return text;
    }
}