#include "save_helper.hpp"

#if defined( _WIN32 )
fs::path save::resolve_root( SaveRoot sr ) {
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
#endif