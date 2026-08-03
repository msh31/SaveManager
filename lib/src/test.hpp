#pragma once

static constexpr unsigned char data[] = {
#embed "test.txt"
};

inline static void testytest( ) { SPDLOG_INFO( "embed size: {}", sizeof( data ) ); }
