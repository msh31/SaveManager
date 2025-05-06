//ReSharper disable InconsistentNaming
namespace SaveManager.Interfaces;

public interface ISaveManager
{
    Task InitializeSaveDetection();
    Task ListSaveGamesAsync();
    Task RenameSaveFilesAsync(string gameId, string saveFileName, string newDisplayName);
}