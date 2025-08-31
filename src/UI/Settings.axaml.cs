using System;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Interactivity;

namespace SaveManagerr;

public partial class SettingsView : UserControl
{
    private readonly ConfigManager _configManager;

    public SettingsView()
    {
        InitializeComponent();
        _configManager = new ConfigManager();
        LoadSettings();
    }

    private void LoadSettings()
    {
        UbisoftToggle.IsChecked = _configManager.Data.Platforms.Ubi;
        UnrealToggle.IsChecked = _configManager.Data.Platforms.Unreal;
        RockstarToggle.IsChecked = _configManager.Data.Platforms.Rockstar;
        
        UbisoftPathText.Text = GetUbisoftPath();
        BackupLocationText.Text = _configManager.Data.BackupLocation;
    }

    private void OnToggleChanged(object sender, RoutedEventArgs e)
    {
        if (sender is ToggleSwitch toggle)
        {
            switch (toggle.Name)
            {
                case "UbisoftToggle":
                    _configManager.Data.Platforms.Ubi = toggle.IsChecked ?? false;
                    break;
                case "UnrealToggle":
                    _configManager.Data.Platforms.Unreal = toggle.IsChecked ?? false;
                    break;
                case "RockstarToggle":
                    _configManager.Data.Platforms.Rockstar = toggle.IsChecked ?? false;
                    break;
            }
            
            _configManager.Save(_configManager.Data);
        }
    }
    
    private void OnBrowseBackupLocation(object sender, RoutedEventArgs e)
    {
        // TODO: Open folder picker dialog
        System.Diagnostics.Process.Start("explorer.exe", _configManager.Data.BackupLocation);
    }

    private string GetUbisoftPath()
    {
        // TODO: Implement
        return @"C:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\savegames";
    }
}