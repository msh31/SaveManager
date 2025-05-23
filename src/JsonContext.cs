﻿using System.Text.Json.Serialization;
using SaveManager.Models;

namespace SaveManager;

[JsonSourceGenerationOptions(WriteIndented = true)]
[JsonSerializable(typeof(ConfigData))]
[JsonSerializable(typeof(List<SaveFileInfo>))]
[JsonSerializable(typeof(Dictionary<string, List<SaveFileInfo>>))]

internal partial class JsonContext : JsonSerializerContext
{
    
}