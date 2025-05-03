using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using MsBox.Avalonia.Enums;
using SaveManagerGUI.core;

namespace SaveManagerGUI;

public partial class MainWindow : Window
{
    private readonly Utils _utils = new();
    
    public MainWindow()
    {
        InitializeComponent();
        _utils.InitializeDiscordRPC();
    }

    private async void OnDashboardClick(object sender, RoutedEventArgs e)
    {
        
    }
    
    private async void OnBackupsClick(object sender, RoutedEventArgs e)
    {
        
    }
    
    private async void OnSyncClick(object sender, RoutedEventArgs e)
    {
        
    }
    
    private async void OnSettingsClick(object sender, RoutedEventArgs e)
    {
        
    }
    
    private async void OnExitClick(object sender, RoutedEventArgs e)
    {
        var result = await _utils.ShowConfirmationBox("Are you sure you want to exit?", "Exit");
    
        if (result == ButtonResult.Yes) Close();
    }
}