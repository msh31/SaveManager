using System;
using System.IO;
using System.Linq;
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

        string detectedPath = null;

        if (OperatingSystem.IsWindows()) {
            detectedPath = @"C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\savegames";
        }
        else if (OperatingSystem.IsLinux()) {
            var possiblePaths = new[] {
                Path.Combine(Globals.homeDir, ".wine", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(Globals.homeDir, ".local", "share", "wineprefixes", "default", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(Globals.homeDir, "Games", "ubisoft-connect", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
            };

            detectedPath = possiblePaths.FirstOrDefault(Directory.Exists);
        }
        else if (OperatingSystem.IsMacOS()) {
            var possiblePaths = new[] {
                Path.Combine(Globals.homeDir, "Library", "Application Support", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(Globals.homeDir, "Library", "Containers", "com.ubisoft.uplay", "Data", "Library", "Application Support", "Ubisoft", "Ubisoft Game Launcher", "savegames")
            };

            detectedPath = possiblePaths.FirstOrDefault(Directory.Exists);
        }

        if (detectedPath != null && Directory.Exists(detectedPath)) {
            _configManager.Data.UbisoftFolderPath = detectedPath;
            _configManager.Save(_configManager.Data);
            return detectedPath;
        }

        return "Not configured";
    }
}