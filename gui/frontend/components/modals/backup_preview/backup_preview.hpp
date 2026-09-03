#pragma once
#include "../base_model.hpp"

class CBackupPreviewModal : public CModalBase {
    public:
        CBackupPreviewModal( ) : CModalBase( "Backup Preview" ) {};

        void open( const std::vector<std::string>& list );
        void render_content( );

    private: 
        std::vector<std::string> m_preview_list = { };
};