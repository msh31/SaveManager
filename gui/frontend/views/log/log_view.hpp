#include <frontend/views/base_view.hpp>

class CLogView : public CBaseView {
    public:
        ~CLogView( ) override = default;
        void render( ) override;

    private:
        bool m_auto_scroll = true;
};
