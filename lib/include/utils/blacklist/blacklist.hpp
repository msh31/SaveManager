#pragma once

struct Blacklist {
    public:
        bool init( );
        void save( );

        bool is_blacklisted( const std::string& game_name ) const;
        const std::unordered_set<std::string>& games( ) const;

        void add( const std::string& name );
        void remove( const std::string& name );

    private:
        std::unordered_set<std::string> m_blacklisted_games;
};
