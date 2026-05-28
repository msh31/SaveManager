#pragma once

class CDialog {
    public:
        virtual ~CDialog( ) = default;

    private:
        std::unordered_set<std::string, bool> m_dialogs;
};
