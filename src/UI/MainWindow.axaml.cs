using Avalonia.Controls;
using Avalonia.Interactivity;

namespace SaveManagerr;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        ShowDashboard();
    }

    private void NavigateTo(object sender, RoutedEventArgs e)
    {
        if (sender is not Button button) {
            return;
        }
        ClearActiveStates();
        button.Classes.Add("active");
        
        switch (button.Name)
        {
            case "DashboardBtn":
                ShowDashboard();
                break;
            case "SavesBtn":
                ShowSaves();
                break;
            case "BackupBtn":
                ShowBackup();
                break;
            case "RestoreBtn":
                ShowRestore();
                break;
            case "SyncBtn":
                ShowSync();
                break;
            case "LogsBtn":
                ShowLogs();
                break;
            case "LoginBtn":
                ShowLogin();
                break;
            case "SettingsBtn":
                ShowSettings();
                break;
        }
    }

    private void ClearActiveStates()
    {
        var navButtons = new[] { DashboardBtn, SavesBtn, BackupBtn, RestoreBtn, SyncBtn, LogsBtn, LoginBtn, SettingsBtn };
        
        foreach (var btn in navButtons) {
            btn.Classes.Remove("active");
        }
    }
    
    private void ShowDashboard()
    {
        MainContent.Content = new DashboardView();
    }

    private void ShowSaves()
    {
        // TODO: Create SavesView
        MainContent.Content = new TextBlock { Text = "Saves view coming soon!", Foreground = Avalonia.Media.Brushes.White };
    }

    private void ShowBackup()
    {
        // TODO: Create BackupView
        MainContent.Content = new TextBlock { Text = "Backup view coming soon!", Foreground = Avalonia.Media.Brushes.White };
    }

    private void ShowRestore()
    {
        // TODO: Create RestoreView
        MainContent.Content = new TextBlock { Text = "Restore view coming soon!", Foreground = Avalonia.Media.Brushes.White };
    }

    private void ShowSync()
    {
        // TODO: Create SyncView
        MainContent.Content = new TextBlock { Text = "Sync view coming soon!", Foreground = Avalonia.Media.Brushes.White };
    }

    private void ShowLogs()
    {
        // TODO: Create LogsView
        MainContent.Content = new TextBlock { Text = "Logs view coming soon!", Foreground = Avalonia.Media.Brushes.White };
    }

    private void ShowLogin()
    {
        // TODO: Create LoginView
        MainContent.Content = new TextBlock { Text = "Login view coming soon!", Foreground = Avalonia.Media.Brushes.White };
    }

    private void ShowSettings()
    {
        MainContent.Content = new SettingsView();
    }
}