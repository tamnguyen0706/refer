/**
  Copyright © 2023 COMPAN REF
  @file company_ref_ws_main.cpp
  @brief Entry point
  @author Stefan Hallas Mulvad
*/

#include "company_ref_ws_main_app.h"

#include <Compan_logger/Compan_logger_sink_cout.h>
#include <Compan_logger/Compan_logger_sink_syslog.h>

using namespace Compan::Edge;

int main(int argc, char* argv[])
{
    CompanLoggerSinkSyslog sinkSysLog(false, 0);
    CompanLoggerSinkCout sinkCout(false, 0);

    MainApp mainApp;
    return mainApp.run(argc, argv);
}
