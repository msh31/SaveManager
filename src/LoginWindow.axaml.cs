using Avalonia.Controls;
using Avalonia.Interactivity;
using MsBox.Avalonia.Enums;
using SaveManagerGUI.core;

namespace SaveManagerGUI;

public partial class LoginWindow : Window
{
    private readonly Utils _utils = new();
    private readonly Auth _auth;
    
    public LoginWindow()
    {
        InitializeComponent();
        _auth = new Auth(_utils);
    }
    
    private async void OnLoginClick(object sender, RoutedEventArgs e)
    {
        var username = UsernameTextBox.Text;
        var password = PasswordTextBox.Text;
        
        if (string.IsNullOrWhiteSpace(username) || string.IsNullOrWhiteSpace(password))
        {
            _utils.ShowMessageBox("Username and password cannot be empty.", "Error");
            return;
        }
        
        await _auth.LoginUser(username, password);
    }
    
    private async void OnExitClick(object sender, RoutedEventArgs e)
    {
        var result = await _utils.ShowConfirmationBox("Are you sure you want to exit?", "Exit Application");
    
        if (result == ButtonResult.Yes) Close();
    }
}