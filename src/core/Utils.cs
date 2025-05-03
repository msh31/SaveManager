using System.Threading.Tasks;
using DiscordRPC;
using DiscordRPC.Logging;
using MsBox.Avalonia;
using MsBox.Avalonia.Enums;

namespace SaveManagerGUI.core;

public class Utils
{
    public DiscordRpcClient client;
    
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

    public async Task InitializeDiscordRPC()
    {
        client = new DiscordRpcClient("1367995954388533489");			
        
        client.Logger = new ConsoleLogger() { Level = LogLevel.Warning };
        
        client.Initialize();
        
        client.SetPresence(new RichPresence()
        {
            Details = "A local savegame manager",
            State = "idle",
            Assets = new Assets()
            {
                LargeImageKey = "image_large",
                LargeImageText = "cloud sync begone"
                // SmallImageKey = "image_small"
            }
        });	
    }
}