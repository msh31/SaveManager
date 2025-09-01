using System;

namespace SaveManager.Core;

public class Globals
{
    // App properties
    public const string AppName = "SaveManager";
    public const string AppVersion = "1.0.0.0";
    
    public static string homeDir = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
}