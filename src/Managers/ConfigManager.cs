//ReSharper disable InconsistentNaming

namespace SaveManager.Managers;

using Models;
using System.IO;
using System.Text.Json;

public class ConfigManager
{
    private readonly string _configFilePath;
    
    public ConfigData Data { get; private set; }
    
    public ConfigManager(string configFilePath)
    {
        _configFilePath = configFilePath;
        Data = new ConfigData();
        Load();
    }
    
    private void Load()
    {
        if (File.Exists(_configFilePath))
        {
            var json = File.ReadAllText(_configFilePath);
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
        File.WriteAllText(_configFilePath, json);
    }
}