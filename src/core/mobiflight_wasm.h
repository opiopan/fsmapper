//
// mobiflight_wasm.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>
#include <SimConnect.h>
#include <sol/sol.hpp>

class FS2020;

void mfwasm_create_lua_env(FS2020& fs2020, sol::table& fs2020_table);
void mfwasm_start(FS2020& fs2020, HANDLE handle);
void mfwasm_stop();
void mfwasm_process_client_data(SIMCONNECT_RECV_CLIENT_DATA* data);
void mfwasm_update_simvar_observation();