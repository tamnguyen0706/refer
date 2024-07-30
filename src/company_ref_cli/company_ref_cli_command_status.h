/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_command_status.h
  @brief CompanEdge - CLI command status
*/

#ifndef __company_ref_CLI_COMMAND_STATUS_H__
#define __company_ref_CLI_COMMAND_STATUS_H__

namespace Compan{
namespace Edge {
enum CommandStatus {
    CMD_STATUS_SUCCESS = 0,    // successful
    CMD_STATUS_FAIL = 1,       // command failed
    CMD_STATUS_IN_PROGRESS = 2 // command successful, continue
};

enum FormatOption { FORMAT_Compan= 0, FORMAT_TR069 };

constexpr const char* Compan_SYSTEM_CONFIG_TIMEZONE = "system.timezone.config.timezone";
} // namespace Edge
} // namespace Compan

#endif
