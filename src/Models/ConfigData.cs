namespace SaveManager.Models;

public class ConfigData
{
    public string DetectedUbiAccount { get; set; } = string.Empty;
    public string DetectedUbiPath { get; set; } = string.Empty;
    public string DetectedRockstarAccount { get; set; } = string.Empty;
    public string DetectedRockstarPath { get; set; } = string.Empty;
    
    public List<string> DetectedUbiGames { get; set; } = [];
    public List<string> DetectedRockstarGames { get; set; } = [];
}