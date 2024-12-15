using System.Windows;
using System.Windows.Input;

namespace SaveManager
{
    public partial class MainWindow : Window
    {
        private bool rockstarSelected = true;
        private bool ubisoftSelected = false;

        public MainWindow()
        {
            InitializeComponent();
            UpdateCheckmarks();
        }

        private void RockstarCard_MouseDown(object sender, MouseButtonEventArgs e)
        {
            rockstarSelected = !rockstarSelected;
            UpdateCheckmarks();
        }

        private void UbisoftCard_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ubisoftSelected = !ubisoftSelected;
            UpdateCheckmarks();
        }

        private void ContinueButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // Here you can handle the selected launchers and navigate to the next window
            MessageBox.Show($"Selected launchers:\nRockstar: {rockstarSelected}\nUbisoft: {ubisoftSelected}");
        }

        private void UpdateCheckmarks()
        {
            // Update Rockstar checkmark
            RockstarCheckmark.Visibility = rockstarSelected ? Visibility.Visible : Visibility.Collapsed;

            // Update Ubisoft checkmark
            UbisoftCheckmark.Visibility = ubisoftSelected ? Visibility.Visible : Visibility.Collapsed;
        }
    }
}