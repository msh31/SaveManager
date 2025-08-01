namespace SaveManager.Models;

public class BackupLogEntry
{
    public string Id { get; set; } = Guid.NewGuid().ToString();
    public DateTime BackupDate { get; set; } = DateTime.Now;
    public List<SaveFileInfo> BackedUpSaves { get; set; } = [];
    public string BackupLocation { get; set; } = string.Empty;
    public long TotalSizeBytes { get; set; }
    public bool Success { get; set; }
    public string ErrorMessage { get; set; }
}

public class BackupSelections
{
    public DateTime LastUpdated { get; set; } = DateTime.Now;
    public List<string> Selections { get; set; } = [];
}