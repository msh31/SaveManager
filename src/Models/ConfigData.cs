﻿namespace SaveManager.Models;

public class ConfigData
{
    public bool FirstRun { get; set; } = true;
    public string DetectedUbiAccount { get; set; } = string.Empty;
    public string DetectedUbiPath { get; set; } = string.Empty;
    public List<string> DetectedUbiGames { get; set; } = new();
    public string DetectedRockstarAccount { get; set; } = string.Empty;
    public List<string> DetectedRockstarGames { get; set; } = new();
}