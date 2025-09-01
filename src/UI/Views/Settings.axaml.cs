using System.IO;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.VisualTree;
using SaveManager.Core;
using SaveManager.Modules;

namespace SaveManagerr;

public partial class SettingsView : UserControl
{
    private readonly ConfigManager _configManager;
    private readonly Ubisoft _ubisoftManager;

    public SettingsView()
    {
        InitializeComponent();
        _configManager = new ConfigManager();
        _ubisoftManager = new Ubisoft(_configManager);
        LoadSettings();
    }

    private void LoadSettings()
    {
        var ubiPath = _ubisoftManager.GetUbisoftPath();
        UbisoftPathText.Text = ubiPath;
        
        UbisoftToggle.IsChecked = (ubiPath != "Not configured") && _configManager.Data.Platforms.Ubi;
        UnrealToggle.IsChecked = _configManager.Data.Platforms.Unreal;
        RockstarToggle.IsChecked = _configManager.Data.Platforms.Rockstar;

        BackupLocationText.Text = _configManager.Data.BackupLocation;
    }

    private async void OnToggleChanged(object sender, RoutedEventArgs e)
    {
        if (sender is not ToggleSwitch toggle) {
            return;
        }

        bool isChecked = toggle.IsChecked.GetValueOrDefault(false);

        switch (toggle.Name)
        {
            case "UbisoftToggle":
                if (isChecked) {
                    var ubiPath = _ubisoftManager.GetUbisoftPath();

                    if (ubiPath == "Not configured") {
                        bool configured = await ShowConfigureUbisoftPathDialogAsync();
                        if (!configured) {
                            toggle.IsChecked = false;
                            return;
                        }
                    }
                    _configManager.Data.Platforms.Ubi = true;
                } else {
                    _configManager.Data.Platforms.Ubi = false;
                }
                UbisoftPathText.Text = _ubisoftManager.GetUbisoftPath();
                break;

            case "UnrealToggle":
                _configManager.Data.Platforms.Unreal = isChecked;
                break;

            case "RockstarToggle":
                _configManager.Data.Platforms.Rockstar = isChecked;
                break;
        }

        _configManager.Save(_configManager.Data);
    }

    private void OnBrowseBackupLocation(object sender, RoutedEventArgs e)
    {
        //TODO
        if (Directory.Exists(_configManager.Data.BackupLocation)) {
            System.Diagnostics.Process.Start("explorer.exe", _configManager.Data.BackupLocation);
        }
    }

    private async Task<bool> ShowConfigureUbisoftPathDialogAsync()
    {
        var dialog = new OpenFolderDialog {
            Title = "Select Ubisoft savegames folder"
        };

        var window = this.GetVisualRoot() as Window;
        var result = await dialog.ShowAsync(window);

        if (string.IsNullOrEmpty(result) || !Directory.Exists(result)) {
            return false;
        }
        
        _configManager.Data.UbisoftFolderPath = result;
        _configManager.Save(_configManager.Data);
        return true;
    }
}