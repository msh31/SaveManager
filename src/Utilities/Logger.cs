using System.Text;
using Spectre.Console;

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
            var logEntry = FormatLogEntry(message, levelName, indentLevel);
            
            lock (_fileLock)
            {
                using var writer = new StreamWriter(_logFilePath, true);
                writer.WriteLine(logEntry);
            }

            if (!consoleOutput) return;
            
            AnsiConsole.Write(new Markup($"[{color}]{logEntry}[/]\n"));
        }
        catch (IOException ex)
        {
            AnsiConsole.Write(new Markup($"[red]Failed to write to log file:[/] {ex.Message}\n"));
        }
        catch (Exception ex)
        {
            AnsiConsole.Write(new Markup($"[red]Unexpected error writing to log:[/] {ex.Message}\n"));
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
}