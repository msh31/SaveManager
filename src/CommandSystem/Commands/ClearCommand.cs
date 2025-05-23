﻿//ReSharper disable InconsistentNaming
//ReSharper disable ConvertToPrimaryConstructor
//ReSharper disable SuggestVarOrType_BuiltInTypes

using SaveManager.Commands;
using Spectre.Console;

class ClearCommand : ICommand
{
    public string Name => "clear";
    public string Description => "Clears the console screen";
    public string Usage => "";
    
    public Task ExecuteAsync(string[] args)
    {
        Console.Clear();
        AnsiConsole.Write(new FigletText("SaveManager").Centered().Color(Color.Cyan1));
        AnsiConsole.Write(Align.Center(new Markup("All clear!\nType [bold yellow]'help'[/] to see available commands!")));
        return Task.CompletedTask;
    }
}