using System;
using System.IO;
using System.Text.Json;

public class AppConfig
{
    public PlatformSupport Platforms { get; set; } = new();
    
    public bool RsyncEnabled { get; set; }
    
    public string Username { get; } = Environment.UserName;
    public string BackupLocation { get; } = Path.Combine(Directory.GetCurrentDirectory(), "Backups");
}

public class PlatformSupport
{
    public bool Unreal { get; set; }
    public bool Ubi { get; set; }
    public bool Rockstar { get; set; }
}

public class ConfigManager
{
    private readonly string _path;
    public AppConfig Data { get; }

    public ConfigManager(string path = "config.json")
    {
        _path = path;
        Data = Load();
    }

    private AppConfig Load()
    {
        if (!File.Exists(_path)) {
            var defaultConfig = new AppConfig();
            Save(defaultConfig);
            return defaultConfig;
        }

        var json = File.ReadAllText(_path);
        var loadedConfig = JsonSerializer.Deserialize<AppConfig>(json);

        if (loadedConfig != null) {
            return loadedConfig;
        }

        return new AppConfig();
    }
    
    public void Save(AppConfig config)
    {
        var options = new JsonSerializerOptions { WriteIndented = true };
        var json = JsonSerializer.Serialize(config, options);
        File.WriteAllText(_path, json);
    }
}