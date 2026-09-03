#pragma once
#include <frontend/views/base_view.hpp>

#include <frontend/components/modals/tags/tags_modal.hpp>

/*
    TODO LIST

    1. replace dummy data with real data
*/

class CHomeView : public CBaseView {
    public:
        ~CHomeView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        CTagsModal m_tags_modal; //1.
};
