#ifndef ARKTS_SCRIPT_PARAM_PARSER_H
#define ARKTS_SCRIPT_PARAM_PARSER_H

#include <string>

#include "napi/native_api.h"

namespace ArkTSScript {

bool ParseArktsScriptParams(
    napi_env env, napi_value jsParam, std::string &cliArgs, std::string &errorMessage);

} // namespace ArkTSScript

#endif // ARKTS_SCRIPT_PARAM_PARSER_H
