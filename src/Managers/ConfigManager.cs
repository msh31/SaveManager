namespace SaveManager.Managers;

using System.IO;
using System.Text.Json;

// holds the data
public class ConfigData
{
    public bool FirstRun { get; set; } = true;
    public bool UbisoftSupport { get; set; } = true;
    public bool RockstarSupport { get; set; } = true;
    public string DetectedUbiAccount { get; set; } = string.Empty;
    public List<string> DetectedUbiGames { get; set; } = new();
    public string DetectedRockstarAccount { get; set; } = "unknown";
    public List<string> DetectedRockstarGames { get; set; } = new();
}

// handles loading/saving
public class ConfigManager
{
    public ConfigData Data { get; private set; }
    
    public ConfigManager()
    {
        Data = new ConfigData();
        Load();
    }
    
    private void Load()
    {
        if (File.Exists(Path.Combine(AppContext.BaseDirectory, "config.json")))
        {
            var json = File.ReadAllText(Path.Combine(AppContext.BaseDirectory, "config.json"));
            var loadedConfig = JsonSerializer.Deserialize<ConfigData>(json);
            
            if (loadedConfig != null)
            {
                Data = loadedConfig;
            }
        }
        else
        {
            Save();
        }
    }
    
    public void Save()
    {
        var json = JsonSerializer.Serialize(Data, new JsonSerializerOptions { WriteIndented = true });
        File.WriteAllText(Path.Combine(AppContext.BaseDirectory, "config.json"), json);
    }
}