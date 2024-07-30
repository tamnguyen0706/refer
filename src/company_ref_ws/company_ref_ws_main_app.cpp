/**
 Copyright © 2024 COMPAN REF
 @file company_ref_ws_main_app.cpp
 @brief Main application class
 */

#include "company_ref_ws_main_app.h"

#include <company_ref_main_apps/company_ref_app_options.h>

#include <company_ref_sdk_value_ids/dynamic_dmo_meta_data.h>
#include <company_ref_sdk_value_ids/logger_meta_data.h>
#include <company_ref_sdk_value_ids/microservices_meta_data.h>

#include <company_ref_main_apps/Compan_logger_factory.h>

// #include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_dmo/company_ref_dmo_helper.h>
#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>

#include <Compan_logger/Compan_logger.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/filesystem.hpp>

#include <thread>

#include <iostream>
#include <vector>

using namespace Compan::Edge;

CompanLogger AeWsMainLog("appmain", LogLevel::Information);

MainApp::MainApp()
    : mainIo_(AeWsMainLog)
    , ws_(mainIo_.getIoContext())
    , dmo_()
    , handlerFactory_(mainIo_.getIoContext(), ws_, dmo_)
    , udsPath_("/tmp/company_ref_ws.socket")
    , loadPath_("")
    , restorePath_("")
    , dmoPath_("")
    , persistedPath_("")
    , enableTcp_(false)
    , threads_(std::thread::hardware_concurrency())
{
}

int MainApp::run(int argc, char* argv[])
{
    appName_ = [&argv]() {
        auto p = strrchr(argv[0], '/');
        return (p ? ++p : argv[0]);
    }();

    if (!parseCmdLine(argc, argv)) return usage(1);

    LoggerMetaData::create(ws_, dmo_);
    DynamicDmoMetaData::create(ws_, dmo_);
    MicroservicesMetaData::create(ws_, dmo_);

    LoggerFactory::createBinders("WsServer", ws_, false);

    controller_ = std::make_unique<DynamicDmoController>(mainIo_.getIoContext(), ws_, dmo_);

    if (!loadDmoFromCmdLine()) {
        controller_->dmoPath(dmoPath_);
        controller_->persistPath(persistedPath_);
    }

    try {

        startUdsServer();
        startTcpServer();

        InfoLog(AeWsMainLog) << "VariantValueStore Server launching: " << threads_ << std::endl;

        mainIo_.run(threads_);

    } catch (std::exception& e) {
        std::cerr << "Error: exception occured: " << e.what() << "\n";
    }

    if (!udsPath_.empty()) { unlink(udsPath_.c_str()); };
    return 0;
}

bool MainApp::parseCmdLine(int argc, char* argv[])
{
    AppOptionsParser appOptionsParser({
            {'u', "uds", true, false},
            {'l', "load", true, false},
            {'r', "restore", true, false},
            {'c', "threads", true, false},
            {'d', "dmo", true, false},
            {'p', "persisted", true, false},
            {'t', "tcp", false, false},
            {'h', "help", false, false},
    });

    if (!appOptionsParser.parse(argc, argv)) return false;

    if (appOptionsParser.has('h')) return false;

    if (appOptionsParser.has('u')) udsPath_ = appOptionsParser.single('u');
    if (appOptionsParser.has('c')) {
        try {
            threads_ = std::max(1, std::min(threads_, std::stoi(appOptionsParser.single('c'))));
        } catch (...) {
        }
    }
    if (appOptionsParser.has('l')) loadPath_ = appOptionsParser.single('l');
    if (appOptionsParser.has('r')) restorePath_ = appOptionsParser.single('r');

    if (appOptionsParser.has('d')) dmoPath_ = appOptionsParser.single('d');
    if (appOptionsParser.has('p')) persistedPath_ = appOptionsParser.single('p');

    if (appOptionsParser.has('t')) enableTcp_ = true;

    return true;
}

int MainApp::usage(int const ret)
{
    std::cout << "Usage: " << appName_ << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -u, --uds <path>      Path to uds socket (ex: --uds=/tmp/company_ref_ws.socket)" << std::endl;
    std::cout << "  -l, --load <path>       Path to data model file " << std::endl;
    std::cout << "  -r, --restore <path>    Path to restore a persisted state " << std::endl;
    std::cout << "  -d, --dmo <path>        Path to data model files" << std::endl;
    std::cout << "  -p, --persisted <path>  Path to persisted files" << std::endl;

    // Hide the tcp option; use primarily by devs
    // std::cout << "  -t, --tcp             Turns on the default tcp server" << std::endl;

    std::cout << "  -h, --help            display this message and exit" << std::endl;
    std::cout << std::endl;
    return ret;
}

bool MainApp::loadDmoFromCmdLine()
{
    DynamicDmoValueIds dynamicDmoIds(ws_);
    DmoValueStoreHelper dmoHelper(dmo_, ws_);

    if (loadPath_.empty() && restorePath_.empty()) return false;

    dmoHelper.insertChild(dynamicDmoIds.dmo->id(), appName_);

    DynamicDmoValueIds::Dmo valueIds(ws_, dynamicDmoIds.dmo->id(), appName_);

    if (!loadPath_.empty()) {
        boost::filesystem::path path = boost::filesystem::path(loadPath_);

        controller_->dmoPath(path.parent_path().generic_string());

        valueIds.config->path->set(path.filename().generic_string());
        valueIds.config->action->set(DynamicDmoValueIds::Dmo::Config::Load);

        mainIo_.getIoContext().run();
        mainIo_.getIoContext().restart();
    }

    if (!restorePath_.empty()) {
        boost::filesystem::path path = boost::filesystem::path(restorePath_);

        controller_->persistPath(path.parent_path().generic_string());

        dmoHelper.insertChild(valueIds.persisted->id(), appName_);
        DynamicDmoValueIds::Dmo::Persisted persistedIds(ws_, valueIds.persisted->id(), appName_);

        persistedIds.config->path->set(path.filename().generic_string());
        persistedIds.config->action->set(DynamicDmoValueIds::Dmo::Config::Load);

        mainIo_.getIoContext().run();
        mainIo_.getIoContext().restart();
    }

    return true;
}

void MainApp::startUdsServer()
{
    if (udsPath_.empty()) return;

    udsServer_ = std::make_unique<AsioUdsServer>(mainIo_.getIoContext(), handlerFactory_, udsPath_);
    DebugLog(AeWsMainLog) << "Server is running, Uds: " << udsPath_ << std::endl;
}

void MainApp::startTcpServer()
{
    if (!enableTcp_) return;

    std::string const host = "0.0.0.0";
    std::string const port = "9998";

    // Start Tcp connection
    boost::system::error_code ec;
    boost::asio::ip::address ipAddress = boost::asio::ip::address::from_string(host, ec);
    if (ec.value() != 0) {
        // Provided IP address is invalid. Breaking execution.
        ErrorLog(AeWsMainLog) << "Failed to parse the IP address " << ipAddress << ":" << port << ", ec=" << ec.value()
                              << " : " << ec.message() << std::endl;
        return;
    }

    tcpServer_ = std::make_unique<AsioTcpServer>(mainIo_.getIoContext(), handlerFactory_, host, port);
    DebugLog(AeWsMainLog) << "Server is running, Ip: " << host << ":" << port << std::endl;
}
