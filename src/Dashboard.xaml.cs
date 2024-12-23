using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using SaveManager.Pages;

namespace SaveManager
{
    public partial class Dashboard : Window
    {
        public Dashboard()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                MainWindowContent.Visibility = Visibility.Collapsed;
                WindowFrame.Navigate(new Setup());
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Navigation error: {ex.Message}");
            }
        }
        
        public void ShowWindowContent()
        {
            MainWindowContent.Visibility = Visibility.Visible;
        }
    }
}