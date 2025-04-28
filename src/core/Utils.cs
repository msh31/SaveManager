using System.Threading.Tasks;
using MsBox.Avalonia;
using MsBox.Avalonia.Enums;

namespace SaveManagerGUI.core;

public class Utils
{
    public async void ShowMessageBox(string message, string title = "Message")
    {
        var box = MessageBoxManager.GetMessageBoxStandard(
            title,
            message,
            ButtonEnum.Ok);
        
        await box.ShowAsync();
    }
    
    public async Task<ButtonResult> ShowConfirmationBox(string message, string title = "Confirm")
    {
        var box = MessageBoxManager.GetMessageBoxStandard(
            title,
            message,
            ButtonEnum.YesNo);
        
        return await box.ShowAsync();
    }
}