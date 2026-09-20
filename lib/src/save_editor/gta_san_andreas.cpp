#include <save_editor/gta_san_andreas.hpp>
#include <logger.hpp>
#include <utils/utils.hpp>

std::uint32_t SanAndreas::calculate_checksum( ) {
    std::uint32_t sum = 0;
    for ( auto i = 0; i < data.size( ) - 4; i++ ) {
        sum += data[i];
    }
    return sum;
}

bool SanAndreas::validate_checksum( ) {
    uint32_t saved;
    if ( data.size( ) < 4 ) return false;
    std::memcpy( &saved, data.data( ) + data.size( ) - 4, 4 );
    return saved == calculate_checksum( );
}

std::string SanAndreas::get_version_string( size_t offset ) {
    if ( offset + 4 > data.size( ) ) {
        SPDLOG_ERROR( "Invalid offset for version string: {}", offset );
        return { };
    }

    std::vector<uint8_t> v_bytes = { };
    for ( size_t i = { }; i < 4; i++ ) {
        if ( offset + i >= data.size( ) ) {
            SPDLOG_ERROR( "Version ID truncated" );
            return { };
        }
        v_bytes.push_back( data[offset + i] );
    }

    std::string version_hex = { };
    for ( size_t i = 0; i < v_bytes.size( ); i++ ) {
        if ( i > 0 ) version_hex += ",";
        version_hex += std::format( "{:02x}", v_bytes[i] );
    }

    auto it = version_strings.find( version_hex );
    if ( it != version_strings.end( ) ) return it->second;
    return "Unknown Version";
}

bool SanAndreas::validate_file( ) {
    if ( data.size( ) != SanAndreas::save_size ) {
        SPDLOG_ERROR( "Invalid file size, expected {} bytes but got {} bytes", SanAndreas::save_size, data.size( ) );
        return false;
    }

    if ( block_offsets.size( ) != SanAndreas::block_count ) {
        SPDLOG_ERROR( "Invalid block count, expected {} but got {}", SanAndreas::block_count, block_offsets.size( ) );
        return false;
    }

    auto v_offset = block_offsets[0];
    if ( get_version_string( v_offset ) == "Unknown Version" ) {
        SPDLOG_ERROR( "Unknown game version!" );
        return false;
    }
    if ( !validate_checksum( ) ) {
        SPDLOG_ERROR( "Invalid checksum, save file may be corrupted!" );
        return false;
    }
    return true;
}

// finds the offsets of "BLOCK" sections within the save file data ( https://gtamods.com/wiki/Saves_(GTA_SA) )
void SanAndreas::find_block_offsets( size_t start_offset ) {
    uint8_t block_signature[5] = { 0x42, 0x4C, 0x4F, 0x43, 0x4B }; // "BLOCK"
    block_offsets.clear( );

    if ( start_offset > data.size( ) ) return;
    if ( data.size( ) < sizeof( block_signature ) ) return;

    size_t offset = start_offset;
    size_t block_index = 0;

    while ( offset < data.size( ) - 4 ) {
        int has_signature = memcmp( data.data( ) + offset, block_signature, 5 );

        if ( has_signature == 0 ) {
            block_offsets[block_index] = offset + 5;
            block_index++;
        }
        offset++;
    }
}

bool SanAndreas::open( fs::path path ) {
    file.open( path, std::ios::binary );
    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Failed to open savegame!" );
        return false;
    }

    data = std::vector<uint8_t>( std::istreambuf_iterator<char>( file ), { } );
    file.close( ); // everything past this point works on the in-memory copy, and save( ) cannot
                   // replace the file on Windows while a read handle is still open on it

    if ( data.empty( ) ) {
        SPDLOG_ERROR( "Failed to load data from savegame!" );
        return false;
    }

    find_block_offsets( );
    if ( validate_file( ) ) {
        SPDLOG_DEBUG( "file validated!" );
    } else {
        SPDLOG_DEBUG( "file failed to validate!" );
        return false;
    }
    SPDLOG_INFO( "parsing savefile: {}", path.filename( ).string( ) );


    if ( !parse_block_zero( ) ) {
        SPDLOG_ERROR( "Failed to parse BLOCK0, aborting!" );
        return false;
    }
    if ( !parse_block_two( ) ) {
        SPDLOG_ERROR( "Failed to parse BLOCK2, aborting!" );
        return false;
    }
    if ( !parse_block_five( ) ) {
        SPDLOG_ERROR( "Failed to parse BLOCK5, aborting!" );
        return false;
    }
    if ( !parse_block_fifteen( ) ) {
        SPDLOG_ERROR( "Failed to parse BLOCK15, aborting!" );
        return false;
    }
    if ( !parse_block_twenty( ) ) {
        SPDLOG_ERROR( "Failed to parse BLOCK20, aborting!" );
        return false;
    }
    if( !parse_block_twenty_four( ) ) {
        SPDLOG_ERROR( "Failed to parse BLOCK24, aborting!" );
        return false;
    }

    return true;
}

void SanAndreas::close( ) {
    if ( file.is_open( ) ) {
        file.close( );
    }
}

bool SanAndreas::save( fs::path path ) {
    if ( !serialize( ) ) return false;
    std::uint32_t checksum = calculate_checksum( );
    std::memcpy( data.data( ) + data.size( ) - 4, &checksum, 4 );

    if ( !utils::atomic_write( path, std::string( data.begin( ), data.end( ) ) ) ) {
        SPDLOG_ERROR( "Failed to write savegame!" );
        return false;
    }
    return true;
}

bool SanAndreas::parse_block_zero( ) {
    auto bz_offset = block_offsets[0];
    if ( bz_offset + 0x138 > data.size( ) ) {
        SPDLOG_ERROR( "Expected data length for Block 0 data was not received" );
        return false;
    }

    save_version = get_version_string( bz_offset );
    auto ptr = reinterpret_cast<const char*>( data.data( ) + bz_offset + 4 );
    save_name = std::string( ptr, strnlen( ptr, 100 ) );
    return true;
}

bool SanAndreas::parse_block_two( ) {
    auto bt_offset = block_offsets[2];
    if ( bt_offset + 0x28 > data.size( ) ) return false;

    std::memcpy( &health, data.data( ) + bt_offset + 0x04 + 0x1C, 4 );
    std::memcpy( &armor, data.data( ) + bt_offset + 0x04 + 0x20, 4 );
    
    return true;
}

bool SanAndreas::parse_block_five( ) {
    auto bf_offset = block_offsets[5];
    if ( bf_offset + 0x06 > data.size( ) ) return false;

    lose_stuff_after_wasted = data[bf_offset + 0x04];
    lose_stuff_after_busted = data[bf_offset + 0x05];

    return true;
}

bool SanAndreas::parse_block_fifteen( ) {
    auto bft_offset = block_offsets[15];
    if ( bft_offset + 0x27 > data.size( ) ) return false;

    std::memcpy( &money, data.data( ) + bft_offset + 4, 4 );
    std::memcpy( &money_displayed, data.data( ) + bft_offset + 0x10, 4 );

    max_health = data[bft_offset + 35];
    max_armor = data[bft_offset + 36];
    free_busted_once = data[bft_offset + 0x25];
    free_wasted_once = data[bft_offset + 0x26];
    infinite_run = data[bft_offset + 0x20];
    fast_reload = data[bft_offset + 0x21];
    fireproof = data[bft_offset + 0x22];

    return true;
}

bool SanAndreas::parse_block_twenty( ) {
    auto bty_offset = block_offsets[20];
    if ( bty_offset + 4 > data.size( ) ) return false;

    std::memcpy( &tag_count, data.data( ) + bty_offset, 4 );
    if ( bty_offset + 4 + tag_count > data.size( ) ) return false;
    tag_statuses.resize( tag_count );
    std::memcpy( tag_statuses.data( ), data.data( ) + bty_offset + 4, tag_count );

    return true;
}

bool SanAndreas::parse_block_twenty_four( ) {
    auto btyf_offset = block_offsets[24];
    if ( btyf_offset + 4 > data.size( ) ) return false;

    std::memcpy( &usj_count, data.data( ) + btyf_offset, 4 );
    size_t bytes_needed = 4 + static_cast<size_t>( usj_count ) * 0x44;
    if ( btyf_offset + bytes_needed > data.size( ) ) return false;

    usj_done.resize( usj_count );
    usj_found.resize( usj_count );

    for ( uint32_t i = 0; i < usj_count; i++ ) {
        size_t jump_offset = btyf_offset + 4 + static_cast<size_t>( i ) * 0x44;
        usj_done[i] = data[jump_offset + 0x40];
        usj_found[i] = data[jump_offset + 0x41];
    }

    return true;
}

bool SanAndreas::serialize( ) {
    // block 0
    auto bz_offset = block_offsets[0];
    if ( bz_offset + 4 + 100 > data.size( ) ) return false;
    std::memcpy(
        data.data( ) + bz_offset + 4, save_name.c_str( ), std::min( save_name.size( ), static_cast<size_t>( 100 ) ) );

    // block 2
    auto bt_offset = block_offsets[2];
    if ( bt_offset + 0x28 > data.size( ) ) return false;
    std::memcpy( data.data( ) + bt_offset + 0x04 + 0x1C, &health, 4 );
    std::memcpy( data.data( ) + bt_offset + 0x04 + 0x20, &armor, 4 );

    // block 5
    auto bf_offset = block_offsets[5];
    if ( bf_offset + 0x06 > data.size( ) ) return false;
    data[bf_offset + 0x04] = lose_stuff_after_wasted;
    data[bf_offset + 0x05] = lose_stuff_after_busted;

    // block 15
    auto bft_offset = block_offsets[15];
    if ( bft_offset + 0x27 > data.size( ) ) return false;
    std::memcpy( data.data( ) + bft_offset + 4, &money, 4 );
    std::memcpy( data.data( ) + bft_offset + 0x10, &money_displayed, 4 );
    data[bft_offset + 35] = static_cast<uint8_t>( max_health );
    data[bft_offset + 36] = static_cast<uint8_t>( max_armor );
    data[bft_offset + 0x20] = infinite_run;
    data[bft_offset + 0x21] = fast_reload;
    data[bft_offset + 0x22] = fireproof;
    data[bft_offset + 0x25] = free_busted_once;
    data[bft_offset + 0x26] = free_wasted_once;

    // block 20
    auto bty_offset = block_offsets[20];
    if ( bty_offset + 4 + tag_count > data.size( ) ) return false;
    std::memcpy( data.data( ) + bty_offset, &tag_count, 4 );
    std::memcpy( data.data( ) + bty_offset + 4, tag_statuses.data( ), tag_count );

    // block 24
    auto btyf_offset = block_offsets[24];
    size_t bytes_needed = 4 + static_cast<size_t>( usj_count ) * 0x44;
    if ( btyf_offset + bytes_needed > data.size( ) ) return false;
    std::memcpy( data.data( ) + btyf_offset, &usj_count, 4 );
    for ( uint32_t i = 0; i < usj_count; i++ ) {
        size_t jump_offset = btyf_offset + 4 + ( i * 0x44 );
        data[jump_offset + 0x40] = usj_done[i];
        data[jump_offset + 0x41] = usj_found[i];
    }

    return true;
}