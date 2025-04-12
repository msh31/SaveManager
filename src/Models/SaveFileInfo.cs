namespace SaveManager.Models;

public class SaveFileInfo
{
    public string FileName { get; set; }
    public long FileSize { get; set; }
    public DateTime LastModified { get; set; }
    public DateTime DateCreated { get; set; }
    public string DisplayName { get; set; }
}