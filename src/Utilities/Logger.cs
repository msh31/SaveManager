using System.Text;

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
    private readonly TerminalUI _terminalUI;
    private readonly string _logFilePath;
    private LogLevel _minimumLogLevel = LogLevel.Info;
    private bool _includeTimestamp = true;
    private readonly object _fileLock = new();
    
    public Logger(string logFilePath, TerminalUI terminalUI)
    {
        _terminalUI = terminalUI;
        _logFilePath = logFilePath;
        Directory.CreateDirectory(Path.GetDirectoryName(logFilePath));
    }
    
    public void SetMinimumLogLevel(LogLevel level)
    {
        _minimumLogLevel = level;
    }
    
    public void Debug(string message, int indentLevel = 0)
    {
        if (_minimumLogLevel <= LogLevel.Debug)
            WriteLogEntry(message, "DEBUG", indentLevel, ConsoleColor.Gray);
    }
    
    public void Info(string message, int indentLevel = 0)
    {
        if (_minimumLogLevel <= LogLevel.Info)
            WriteLogEntry(message, "INFO", indentLevel, ConsoleColor.White);
    }
    
    public void Warning(string message, int indentLevel = 0)
    {
        if (_minimumLogLevel <= LogLevel.Warning)
            WriteLogEntry(message, "WARN", indentLevel, ConsoleColor.Yellow);
    }
    
    public void Error(string message, int indentLevel = 0)
    {
        if (_minimumLogLevel <= LogLevel.Error)
            WriteLogEntry(message, "ERROR", indentLevel, ConsoleColor.Red);
    }
    
    public void Fatal(string message, int indentLevel = 0)
    {
        if (_minimumLogLevel <= LogLevel.Fatal)
            WriteLogEntry(message, "FATAL", indentLevel, ConsoleColor.DarkRed);
    }
    
    private void WriteLogEntry(string message, string levelName, int indentLevel, ConsoleColor color)
    {
        try
        {
            var logEntry = FormatLogEntry(message, levelName, indentLevel);
            
            lock (_fileLock)
            {
                using (var writer = new StreamWriter(_logFilePath, true))
                {
                    writer.WriteLine(logEntry);
                }
            }
        
            _terminalUI.WriteTextWithColor(logEntry, color, true, false);
        }
        catch (IOException ex)
        {
            _terminalUI.WriteTextWithColor($"Failed to write to log file: {ex.Message}", ConsoleColor.Red, true, false);
        }
        catch (Exception ex)
        {
            _terminalUI.WriteTextWithColor($"Unexpected error writing to log: {ex.Message}", ConsoleColor.Red, true, false);
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