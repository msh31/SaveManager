using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;

namespace SaveManager
{
    public partial class MainWindow : Window
    {
        private bool rockstarSelected = false;
        private bool ubisoftSelected = false;

        public MainWindow()
        {
            InitializeComponent();
            UserNameText.Text = Environment.UserName;
        }

        // toggle the selected/unselected state for cards
        private void ToggleCardState(object sender, MouseButtonEventArgs e)
        {
            Border card = sender as Border;
            if (card == null) return;
            
            if (card.Name == "RockstarCard")
            {
                rockstarSelected = !rockstarSelected;
                UpdateCardVisuals(rockstarSelected, card);
            }
            else if (card.Name == "UbisoftCard")
            {
                ubisoftSelected = !ubisoftSelected;
                UpdateCardVisuals(ubisoftSelected, card);
            }
        }

        // update the look of the card when selected or unselected
        private void UpdateCardVisuals(bool isSelected, Border card)
        {
            Color selectedColor = Color.FromArgb(255, 100, 155, 255); 
            Color unselectedColor = Color.FromArgb(255, 230, 230, 230);

            if (isSelected)
            {
                // highlight border and make it thicker when selected
                AnimateBorderBrush(card, selectedColor); 

                // make the border thicker for emphasis
                AnimateBorderThickness(card, new Thickness(3));

                // change background for a smooth transition
                AnimateBackgroundColor(card, Colors.WhiteSmoke);
            }
            else
            {
                // reset border color back to the original
                AnimateBorderBrush(card, unselectedColor); 

                // set the border thickness back to normal
                AnimateBorderThickness(card, new Thickness(1)); 

                // restore background to original color
                AnimateBackgroundColor(card, Color.FromArgb(255, 245, 245, 245));
            }
        }

        // animate border color change
        private void AnimateBorderBrush(Border card, Color targetColor)
        {
            SolidColorBrush borderBrush = new SolidColorBrush(targetColor);
            ColorAnimation colorAnimation = new ColorAnimation
            {
                To = targetColor,
                Duration = new Duration(TimeSpan.FromMilliseconds(300)),
                AutoReverse = false
            };
            borderBrush.BeginAnimation(SolidColorBrush.ColorProperty, colorAnimation);
            card.BorderBrush = borderBrush;
        }

        // animate the thickness of the border
        private void AnimateBorderThickness(Border card, Thickness targetThickness)
        {
            ThicknessAnimation thicknessAnimation = new ThicknessAnimation
            {
                To = targetThickness,
                Duration = new Duration(TimeSpan.FromMilliseconds(300)),
                AutoReverse = false
            };
            card.BeginAnimation(Border.BorderThicknessProperty, thicknessAnimation);
        }

        // animate background color change
        private void AnimateBackgroundColor(Border card, Color targetColor)
        {
            ColorAnimation backgroundColorAnimation = new ColorAnimation
            {
                To = targetColor,
                Duration = new Duration(TimeSpan.FromMilliseconds(300)),
                AutoReverse = false
            };
            card.Background.BeginAnimation(SolidColorBrush.ColorProperty, backgroundColorAnimation);
        }

        // button click for 'Continue'
        private void ContinueButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // (dev purposes only)
            MessageBox.Show($"Selected launchers:\nRockstar: {rockstarSelected}\nUbisoft: {ubisoftSelected}");
        }
    }
}