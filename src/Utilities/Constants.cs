using System;

public class Constants
{
    // App properties
    public const string AppName = "SaveManager";
    public const string AppVersion = "1.0.0";
    public const string AppDescription = "A tool for managing game saves, the proper way.";
    public const string AppAuthor = "Marco H.";
    public const string AppWebsite = "https://marco007.dev/savemanager"; // not online yet
    public const string AppLicense = "MIT License";
    
    // Folder paths
    public static string AppDataFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), AppName);
    
    // File paths
    public string ConfigFilePath = Path.Combine(AppDataFolder, "config.json");
    public string LogFilePath = Path.Combine(AppDataFolder, "app.log");
}