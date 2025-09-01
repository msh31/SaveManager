using System;
using System.IO;
using SaveManager.Core;

namespace SaveManager.Modules;

public class Ubisoft
{
    private readonly ConfigManager _configManager;
    
    public Ubisoft(ConfigManager configManager)
    {
        _configManager = configManager ?? throw new ArgumentNullException(nameof(configManager));
    }
    
    public string GetUbisoftPath()
    {
        if (!string.IsNullOrEmpty(_configManager.Data.UbisoftFolderPath) && 
            Directory.Exists(_configManager.Data.UbisoftFolderPath)) {
            return _configManager.Data.UbisoftFolderPath;
        }

        return "Not configured";
    }
}