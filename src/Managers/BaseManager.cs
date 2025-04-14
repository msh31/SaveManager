//ReSharper disable InconsistentNaming
using SaveManager.Interfaces;

// base class for platform managers
abstract class BaseManager : ISaveManager
{
    protected readonly TerminalUI TerminalUI;
    protected readonly Globals Globals;
    protected readonly string BackupDirectory;
    protected readonly string PublisherName;
    
    protected BaseManager(TerminalUI terminalUI, Globals globals, string publisherName)
    {
        TerminalUI = terminalUI;
        Globals = globals;
        PublisherName = publisherName;
        
        BackupDirectory = Path.Combine(globals.BackupsFolder, publisherName);
        Directory.CreateDirectory(BackupDirectory);
    }
    
    // abstract methods that all managers must implement
    public abstract Task InitializeSaveDetection();
    public abstract Task ListSaveGamesAsync();
    public abstract Task RenameSaveFilesAsync(string id);
    public abstract Task BackupSaveGame(string gameId, int saveIndex);
    public abstract Task RestoreSaveGame(string backupId);
    
    // Common utilities that all managers can use
    protected Task<bool> CreateBackupDirectory(string gameId)
    {
        var path = Path.Combine(BackupDirectory, gameId);
        Directory.CreateDirectory(path);
        return Task.FromResult(true);
    }
    
    protected async Task<string> CreateBackup(string sourcePath, string gameId, string saveName)
    {
        if (!File.Exists(sourcePath))
        {
            TerminalUI.WriteFormattedTextByType($"Source file not found: {sourcePath}", "err", true, false);
            return null;
        }
        
        await CreateBackupDirectory(gameId);
        var backupPath = Path.Combine(BackupDirectory, gameId, $"{saveName}_{DateTime.Now:yyyyMMdd_HHmmss}.zip");
        
        try
        {
            File.Copy(sourcePath, backupPath);
            return backupPath;
        }
        catch (Exception ex)
        {
            TerminalUI.WriteFormattedTextByType($"Backup failed: {ex.Message}", "err", true, false);
            return null;
        }
    }
}