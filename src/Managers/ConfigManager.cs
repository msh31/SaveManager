//ReSharper disable InconsistentNaming

namespace SaveManager.Managers;

using Models;
using System.IO;
using System.Text.Json;

public class ConfigManager
{
    private readonly Globals _globals;
    
    public ConfigData Data { get; private set; }
    
    public ConfigManager()
    {
        Data = new ConfigData();
        Load();
    }
    
    private void Load()
    {
        if (File.Exists(_globals.ConfigFilePath))
        {
            var json = File.ReadAllText(_globals.ConfigFilePath);
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
        File.WriteAllText(_globals.ConfigFilePath, json);
    }
}