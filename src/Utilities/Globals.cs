//ReSharper disable InconsistentNaming

using System.Runtime.InteropServices;
using SaveManager.Managers;

public class Globals
{
    // General Folder paths
    private readonly string Documents;
    public string UbisoftRootFolder;
    // public readonly string rockstarRootFolder;
    
    // Program Folder paths
    private readonly string HomeDirectory;
    private readonly string LogsFolder;
    private readonly string DataFolder;
    public readonly string BackupsFolder;
    
    // Program File paths
    public readonly string ConfigFilePath;
    public readonly string LogFilePath;
    public readonly string UbiSaveInfoFilePath;
    public readonly string BackupLogFilePath;
    public readonly string BackupSelectionsFilePath;
    
    // Constants
    public const string DefaultDisplayName = "CUSTOM_NAME_NOT_SET";
    public const string SaveFileExtension = ".save";
    public const string OptionsFilePattern = "[Options]";
    public const int MinimumAccountIdLength = 20;
    public const string AllAccountsOption = "all";
    public const double BytesToKb = 1024.0;
    
    // Messages
    public const string NoAccountsFoundMessage = "[red][[err]][/] No Ubisoft accounts found!";
    public const string ProcessingAccountMessage = "[cyan][[inf]][/] Processing account: {0}";
    public const string LookingForSavesMessage = "[cyan][[inf]][/] Looking for savegames..";
    
    public Globals()    {
        // Documents = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
        // rockstarRootFolder = Path.Combine(Documents, "Rockstar Games"); // works when documents folder is placed on another drive :steamhappy:

        HomeDirectory = GetApplicationDataFolder();
        UbisoftRootFolder = GetUbisoftRootFolder();
        
        LogsFolder = Path.Combine(HomeDirectory, "logs");
        DataFolder = Path.Combine(HomeDirectory, "data");
        BackupsFolder = Path.Combine(DataFolder, "backups");
        
        ConfigFilePath = Path.Combine(DataFolder, "config.json");
        LogFilePath = Path.Combine(LogsFolder, "oops.log");
        UbiSaveInfoFilePath = Path.Combine(DataFolder, "ubisoft_save_information.json");
        BackupLogFilePath = Path.Combine(LogsFolder, "backup_log.json");
        BackupSelectionsFilePath = Path.Combine(DataFolder, "backup_selections.json");        
        
        Directory.CreateDirectory(HomeDirectory);
        Directory.CreateDirectory(LogsFolder);
        Directory.CreateDirectory(DataFolder);
        Directory.CreateDirectory(BackupsFolder);
    }
    
    public void UpdateConfig(ConfigManager configManager)
    {
        configManager.Data.DetectedUbiPath = UbisoftRootFolder;
        configManager.Save();
    }
    
    private static string GetUbisoftRootFolder()
    {
        string homeDir = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);

        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) {
            return @"C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\savegames";
        }

        if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) {
            var possiblePaths = new[] {
                Path.Combine(homeDir, ".wine", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(homeDir, ".local", "share", "wineprefixes", "default", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(homeDir, "Games", "ubisoft-connect", "drive_c", "Program Files (x86)", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
            };

            foreach (var path in possiblePaths) {
                if (Directory.Exists(path)) {
                    return path;
                }
            }
        }

        if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX)) {
            var possiblePaths = new[] {
                Path.Combine(homeDir, "Library", "Application Support", "Ubisoft", "Ubisoft Game Launcher", "savegames"),
                Path.Combine(homeDir, "Library", "Containers", "com.ubisoft.uplay", "Data", "Library", "Application Support", "Ubisoft", "Ubisoft Game Launcher", "savegames")
            };

            foreach (var path in possiblePaths) {
                if (Directory.Exists(path)) {
                    return path;
                }
            }
        }

        Console.Error.WriteLine("[error] Ubisoft savegames folder not found.");
        return string.Empty;
    }

    private static string GetApplicationDataFolder()
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) {
            return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "SaveManager");
        }

        if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux) || RuntimeInformation.IsOSPlatform(OSPlatform.OSX)) {
            string homeDir = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            return Path.Combine(homeDir, ".config", "SaveManager");
        }

        throw new PlatformNotSupportedException("Unsupported operating system.");
    }
}