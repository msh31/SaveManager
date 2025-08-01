namespace SaveManager.Models;

public class SaveFileInfo
{
    public string FileName { get; set; } = string.Empty;
    public string FilePath { get; set; } = string.Empty;
    public string GameId { get; set; } = string.Empty;
    public string AccountId { get; set; } = string.Empty;
    public string DisplayName { get; set; } = string.Empty;
    public long FileSizeBytes { get; set; }
    public DateTime LastModified { get; set; }
    
    public string GameName { get; set; }
    public string BackupPath { get; set; }
}