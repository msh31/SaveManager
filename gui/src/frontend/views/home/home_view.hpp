#pragma once
#include <frontend/views/base_view.hpp>

class CHomeView : public CBaseView {
    public:
        ~CHomeView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        std::string m_input_str       = { };
        double      m_input_double    = 0.0;
        float       m_input_float     = 0.0f;
        int         m_input_int       = 0;
        bool        m_checkbox_toggle = false;

        std::vector<std::string> m_fruits         = { "apple", "orange", "strawberry", "banana", "lemon" };
        std::string              m_selected_fruit = m_fruits[0];

        float m_slider_step_f = 10.0f;
        float m_slider_min_f  = 1.0f;
        float m_slider_max_f  = 100.0f;

        int m_slider_step_i = 1;
        int m_slider_min_i  = 1;
        int m_slider_max_i  = 100;

        float m_color[4] = { 1.f, 1.f, 1.f, 1.f };

        std::vector<std::string> m_meats         = { "beef", "pork", "chicken", "fish" };
        std::string              m_selected_meat = m_meats[0];

        bool m_radio_toggle = true;
};
