//ReSharper disable InconsistentNaming
using System;
using System.Runtime.InteropServices;
using SaveManager.Managers;

public class Globals
{
    // General Folder paths
    private readonly string Documents;
    public string UbisoftRootFolder;
    public readonly string rockstarRootFolder;
    
    // Program Folder paths
    private readonly string AppDataFolder;
    private readonly string LogsFolder;
    private readonly string DataFolder;
    public readonly string BackupsFolder;
    
    // Program File paths
    public readonly string ConfigFilePath;
    public readonly string LogFilePath;
    public readonly string UbiSaveInfoFilePath;
    
    public Globals()
    {
        Documents = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
        rockstarRootFolder = Path.Combine(Documents, "Rockstar Games"); // works when documents folder is placed on another drive :steamhappy:
        
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            UbisoftRootFolder = @"C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\savegames";
            AppDataFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "SaveManager");
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
        {
            var homeDir = Environment.GetEnvironmentVariable("HOME");
            AppDataFolder = Path.Combine(homeDir, ".config", "SaveManager");
            
            var possiblePaths = new[]
            {
                Path.Combine(homeDir, ".wine", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(homeDir, ".local", "share", "wineprefixes", "default", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(homeDir, "Games", "ubisoft-connect", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
            };

            foreach (var path in possiblePaths)
            {
                if (Directory.Exists(path))
                {
                    UbisoftRootFolder = path;
                    break;
                }
            }
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
        {
            var homeDir = Environment.GetEnvironmentVariable("HOME");
            AppDataFolder = Path.Combine(homeDir, ".config", "SaveManager");
            
            var possiblePaths = new[]
            {
                Path.Combine(homeDir, "Library", "Application Support", "Ubisoft", "Ubisoft Game Launcher",
                    "savegames"),
                Path.Combine(homeDir, "Library", "Containers", "com.ubisoft.uplay", "Data", "Library",
                    "Application Support", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(homeDir, "Documents", "Ubisoft", "savegames") // Fallback
            };

            foreach (var path in possiblePaths)
            {
                if (Directory.Exists(path))
                {
                    UbisoftRootFolder = path;
                    break;
                }
            }
            
            if (UbisoftRootFolder == null)
            {
                UbisoftRootFolder = Path.Combine(homeDir, "Documents", "SaveManager", "DummyUbisoftSaves");
                Directory.CreateDirectory(UbisoftRootFolder);
            }
        }

        LogsFolder = Path.Combine(AppDataFolder, "logs");
        DataFolder = Path.Combine(AppDataFolder, "data");
        BackupsFolder = Path.Combine(DataFolder, "backups");
        
        ConfigFilePath = Path.Combine(DataFolder, "config.json");
        LogFilePath = Path.Combine(LogsFolder, "oops.log");
        UbiSaveInfoFilePath = Path.Combine(DataFolder, "ubi_save_info.json");
        
        Directory.CreateDirectory(AppDataFolder);
        Directory.CreateDirectory(LogsFolder);
        Directory.CreateDirectory(DataFolder);
    }
    
    public void UpdateConfig(ConfigManager configManager)
    {
        configManager.Data.DetectedUbiPath = UbisoftRootFolder;
        configManager.Save();
    }
}