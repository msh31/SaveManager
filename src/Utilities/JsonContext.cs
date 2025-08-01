using System.Text.Json.Serialization;
using SaveManager.Models;

[JsonSourceGenerationOptions(WriteIndented = true)]
[JsonSerializable(typeof(ConfigData))]
[JsonSerializable(typeof(List<SaveFileInfo>))]
[JsonSerializable(typeof(Dictionary<string, List<SaveFileInfo>>))]
[JsonSerializable(typeof(List<BackupLogEntry>))]
[JsonSerializable(typeof(BackupLogEntry))]
[JsonSerializable(typeof(BackupSelections))]
internal partial class JsonContext : JsonSerializerContext
{
    
}