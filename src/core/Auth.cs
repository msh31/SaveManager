using System;
using System.Linq;
using System.Threading.Tasks;
using MySql.Data.MySqlClient;

namespace SaveManagerGUI.core;

public class Auth
{
    private string _connectionString = "Server=localhost;Port=3306;Database=php_auth_db;User=user;Password=password;";
    private readonly Utils _utils;

    public Auth(Utils utils)
    {
        _utils = utils;
    }
    
    private async Task<MySqlConnection> GetConnectionAsync()
    {

        try
        {
            var conn = new MySqlConnection(_connectionString);
            await conn.OpenAsync();
            return conn;
        }
        catch (Exception ex)
        {
            _utils.ShowMessageBox("❌ Failed to connect to database.", "error");
            Console.WriteLine(ex.Message);
            return null;
        }
    }

    public async Task LoginUser(string username, string password)
    {
        var conn = await GetConnectionAsync();

        try
        {
            var cmd = new MySqlCommand("SELECT password FROM users WHERE username = @username", conn);
            cmd.Parameters.AddWithValue("@username", username);

            var storedHash = (string)await cmd.ExecuteScalarAsync();

            if (storedHash is null)
            {
                _utils.ShowMessageBox("❌ Invalid credentials.", "error");
                return;
            }

            if (VerifyPassword(password, storedHash))
            {
                _utils.ShowMessageBox("✅ Login successful!", "success");
            }
            else
            {
                _utils.ShowMessageBox("❌ Invalid credentials.", "error");
            }
        }
        finally
        {
            await conn.CloseAsync();
        }
    }
    
    private bool IsStrongPassword(string password)
    {
        // At least 8 characters, one uppercase, one lowercase, one number, one special char
        return password.Length >= 8 &&
               password.Any(char.IsUpper) &&
               password.Any(char.IsLower) &&
               password.Any(char.IsDigit) &&
               password.Any(c => !char.IsLetterOrDigit(c));
    }

    private string HashPassword(string password)
    {
        return BCrypt.Net.BCrypt.HashPassword(password);
    }

    private static bool VerifyPassword(string password, string storedHash)
    {
        return BCrypt.Net.BCrypt.Verify(password, storedHash);
    }
}