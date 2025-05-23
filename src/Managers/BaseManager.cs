﻿//ReSharper disable InconsistentNaming
using SaveManager.Interfaces;
using Spectre.Console;

// base class for platform managers
abstract class BaseManager : ISaveManager
{
    protected readonly Globals Globals;
    protected readonly string BackupDirectory;
    protected readonly string PublisherName;
    protected readonly Logger _logger;
    
    protected BaseManager(Globals globals, string publisherName, Logger logger)
    {
        Globals = globals;
        PublisherName = publisherName;
        _logger = logger;
        
        BackupDirectory = Path.Combine(globals.BackupsFolder, publisherName);
        Directory.CreateDirectory(BackupDirectory);
    }
    
    // abstract methods that all managers must implement
    public abstract Task InitializeSaveDetection();
    public abstract Task ListSaveGamesAsync();
    public abstract Task RenameSaveFilesAsync(string gameId, string saveFileName, string newDisplayName);
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
            AnsiConsole.MarkupLine($"[red][[err]][/] Source file not found: {Markup.Escape(sourcePath)}");
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
            AnsiConsole.MarkupLine($"[red][[err]][/] Backup failed: {ex.Message}");
            return null;
        }
    }
}