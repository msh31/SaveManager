//ReSharper disable InconsistentNaming
namespace SaveManager.Managers;

using SaveManager;

// base class for platform managers
public class BaseManager
{
    private readonly TerminalUI _terminalUI;
    private readonly string _savesDirectory;
    //private readonly string _backupDirectory;
    
    public BaseManager(TerminalUI terminalUI, string publisherName)
    {
        _terminalUI = terminalUI;
        
        //_savesDirectory = Path.Combine(Constants.AppDataFolder, publisherName, "Saves");
        //_backupDirectory = Path.Combine(appDataPath, "SaveManager", publisherName, "Backups");
        
        // Ensure directories exist
        // Directory.CreateDirectory(_savesDirectory);
        // Directory.CreateDirectory(_backupDirectory);
    }
    
    public string CreateTimestampedFilename(string baseFilename)
    {
        return $"{baseFilename}_{DateTime.Now:yyyyMMdd_HHmmss}";
    }
    
    public string[] GetAllSaveFiles(string searchPattern)
    {
        try
        {
            return Directory.GetFiles(_savesDirectory, searchPattern);
        }
        catch
        {
            _terminalUI.WriteFormattedTextByType("Failed to get save files", "err", true, false);
            return new string[0];
        }
    }
    
    public string[] GetAllBackupFiles()
    {
        try
        {
            return Directory.GetFiles(_backupDirectory, "*.backup");
        }
        catch
        {
            _terminalUI.WriteFormattedTextByType("Failed to get backup files", "err", true, false);
            return new string[0];
        }
    }
    
    // Common operations
    public bool ExportSave(string savePath, string destinationPath)
    {
        try
        {
            File.Copy(savePath, destinationPath, true);
            _terminalUI.WriteFormattedTextByType($"Save exported to {destinationPath}", "suc", true, false);
            return true;
        }
        catch
        {
            _terminalUI.WriteFormattedTextByType("Export failed", "err", true, false);
            return false;
        }
    }
    
    public bool ImportSave(string sourcePath, string saveId)
    {
        try
        {
            if (!File.Exists(sourcePath))
            {
                _terminalUI.WriteFormattedTextByType($"Source file not found: {sourcePath}", "err", true, false);
                return false;
            }
            
            string destPath = Path.Combine(_savesDirectory, $"{saveId}.save");
            File.Copy(sourcePath, destPath, true);
            _terminalUI.WriteFormattedTextByType($"Save imported as {saveId}", "suc", true, false);
            return true;
        }
        catch
        {
            _terminalUI.WriteFormattedTextByType("Import failed", "err", true, false);
            return false;
        }
    }
}