//ReSharper disable InconsistentNaming
using System.Text.Json;
using SaveManager.Models;

namespace SaveManager.Managers;

public class ConfigManager
{
    private readonly string _configFilePath;
    
    public ConfigData Data { get; private set; }
    
    public ConfigManager(string configFilePath) {
        _configFilePath = configFilePath;
        Data = new ConfigData();
        Load();
    }
    
    private void Load()
    {
        if (File.Exists(_configFilePath)) {
            try {
                var json = File.ReadAllText(_configFilePath);
                
                var options = new JsonSerializerOptions { 
                    WriteIndented = true,
                    TypeInfoResolver = JsonContext.Default 
                };
                
                var loadedConfig = JsonSerializer.Deserialize<ConfigData>(json, options);
            
                if (loadedConfig != null) {
                    Data = loadedConfig;
                }
            }
            catch (Exception ex) {
                Console.WriteLine($"Error loading config: {ex.Message}");
                Save();
            }
        } else {
            Save();
        }
    }

    public void Save()
    {
        try {
            var options = new JsonSerializerOptions { 
                WriteIndented = true,
                TypeInfoResolver = JsonContext.Default 
            };
            
            var json = JsonSerializer.Serialize(Data, options);
            
            File.WriteAllText(_configFilePath, json);
        } catch (Exception ex) {
            Console.WriteLine($"Error saving config: {ex.Message}");
        }
    }
}