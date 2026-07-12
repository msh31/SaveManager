#include <frontend/views/base_view.hpp>

class CLogView : public CBaseView {
    public:
        ~CLogView( ) override = default;
        void render( ) override;
        void on_enter( ) override {}
        void on_exit( ) override {}

    private:
        bool m_auto_scroll = true;
};
