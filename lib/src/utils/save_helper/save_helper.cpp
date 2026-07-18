#include "save_helper.hpp"

fs::path save::resolve_root( SaveRoot sr ) {
    switch ( sr ) {
    case SaveRoot::DOCUMENTS:
        return paths::documents_dir( ); //
        break;
#if defined( _WIN32 )
    case SaveRoot::LOCAL_APPDATA:
        // return paths::get_known_folder_path( FolderID_AppData ) break;
    case SaveRoot::LOCAL_APPDATA_LOW:
        // return paths::get_known_folder_path( FOLDERID_AppDataProgramData ) break;
        break;
    case SaveRoot::PROGRAM_DATA:
        return paths::get_known_folder_path( FOLDERID_AppDataProgramData ) break;
        break;
    case SaveRoot::SAVED_GAMES:
        return paths::get_known_folder_path( FOLDERID_SavedGames ) break;
        break;
#endif
    default:
        return { };
        break;
    }

    return { };
}
