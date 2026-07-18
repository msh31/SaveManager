#include "save_helper.hpp"

//Windows only
fs::path save::resolve_root( SaveRoot sr ) {
#if not defined( _WIN32 )
    throw std::runtime_error( "resolve_root was called on a non Windows platform, this should not happen!" );
#endif

    switch ( sr ) {
    case SaveRoot::DOCUMENTS:
        return paths::get_known_folder_path( FOLDERID_Documents );
        break;
    case SaveRoot::LOCAL_APPDATA:
        return paths::get_known_folder_path( FOLDERID_LocalAppData );
    case SaveRoot::LOCAL_APPDATA_LOW:
        return paths::get_known_folder_path( FOLDERID_LocalAppDataLow );
        break;
    case SaveRoot::PROGRAM_DATA:
        return paths::get_known_folder_path( FOLDERID_ProgramData );
        break;
    case SaveRoot::SAVED_GAMES:
        return paths::get_known_folder_path( FOLDERID_SavedGames );
        break;
    }

    return { };
}