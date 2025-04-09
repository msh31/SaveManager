//ReSharper disable InconsistentNaming

namespace SaveManager.Managers;

using Models;
using System.IO;
using System.Text.Json;

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