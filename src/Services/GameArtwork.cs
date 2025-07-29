using System.Text.Json;

namespace SaveManager.Services;

public class GameArtwork : IDisposable
{
    private readonly HttpClient httpClient;
    private readonly Dictionary<string, string> cache;
    private readonly bool isEnabled;

    public GameArtwork(string apiKey) {
        isEnabled = !string.IsNullOrEmpty(apiKey);
        cache = new Dictionary<string, string>();
        httpClient = new HttpClient();
        
        if (isEnabled) {
            httpClient.DefaultRequestHeaders.Add("Authorization", $"Bearer {apiKey}");
            httpClient.Timeout = TimeSpan.FromSeconds(10);
        }
    }

    public async Task<string> GetGameArtworkAsync(string gameName) {
        if (!isEnabled || string.IsNullOrEmpty(gameName)) {
            return null;
        }

        // Check cache first
        if (cache.ContainsKey(gameName)) {
            return cache[gameName];
        }

        try {
            // Search for the game
            var gameId = await SearchGameAsync(gameName);
            if (gameId > 0) {
                // Get hero image
                var artworkUrl = await GetHeroImageAsync(gameId);
                cache[gameName] = artworkUrl;
                return artworkUrl;
            }
        } catch {
            // Silently fail
        }

        cache[gameName] = null;
        return null;
    }

    private async Task<int> SearchGameAsync(string gameName) {
        try {
            var searchUrl = $"https://www.steamgriddb.com/api/v2/search/autocomplete/{Uri.EscapeDataString(gameName)}";
            var response = await httpClient.GetStringAsync(searchUrl);
            var result = JsonSerializer.Deserialize<SearchResult>(response);

            if (result != null && result.data != null && result.data.Length > 0) {
                return result.data[0].id;
            }
        } catch {
            // Silently fail
        }

        return 0;
    }

    private async Task<string> GetHeroImageAsync(int gameId) {
        try {
            var heroUrl = $"https://www.steamgriddb.com/api/v2/heroes/game/{gameId}";
            var response = await httpClient.GetStringAsync(heroUrl);
            var result = JsonSerializer.Deserialize<ArtworkResult>(response);

            if (result != null && result.data != null && result.data.Length > 0) {
                return result.data[0].url;
            }
        } catch {
            // Silently fail
        }

        return null;
    }

    public void Dispose()
    {
        httpClient?.Dispose();
    }
}

// Simple data classes
public class SearchResult
{
    public GameInfo[] data { get; set; }
}

public class GameInfo
{
    public int id { get; set; }
    public string name { get; set; }
}

public class ArtworkResult
{
    public ArtworkInfo[] data { get; set; }
}

public class ArtworkInfo
{
    public string url { get; set; }
}