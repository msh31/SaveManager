//ReSharper disable InconsistentNaming

using System.Text.Json;
using System.Collections.Concurrent;
using Spectre.Console;

internal class Utilities
{
    private readonly Logger _logger;
    
    public Utilities(Logger logger)
    {
        _logger = logger;
    }
    
    private ConcurrentDictionary<string, string> gameNames;
    private readonly HttpClient client = new();
    
    public async Task<string> TranslateUbisoftGameId(string gameFolder)
    {
        await LoadGameTranslations();

        var gameId = Path.GetFileName(gameFolder);

        if (gameNames.TryGetValue(gameId, out string fullName))
        {
            return fullName;
        }

        return gameId;
    }
    
    private async Task LoadGameTranslations()
    {
        try
        {
            var json = await client.GetStringAsync("https://raw.githubusercontent.com/msh31/Ubisoft-Game-Ids/refs/heads/master/gameids.json");
            var franchises = JsonSerializer.Deserialize<Dictionary<string, Dictionary<string, string>>>(json);

            if (franchises != null)
            {
                gameNames = new ConcurrentDictionary<string, string>();
                foreach (var franchise in franchises.Values)
                {
                    foreach (var game in franchise)
                    {
                        gameNames[game.Key] = game.Value;
                    }
                }
            }
        }
        catch (Exception ex)
        {
            _logger.Fatal(ex.Message, 2, true);
            gameNames = new ConcurrentDictionary<string, string>();
        }
    }
}