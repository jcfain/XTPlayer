#include "mainwindow.h"

#include <QApplication>
#include "lib/tool/qsettings_json.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{

#ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);
    } else {
        printf("fail to AttachConsole(ATTACH_PARENT_PROCESS)\n");
    }
#endif

    QCoreApplication *a;
    bool consoleMode = false;
    bool verboseMode = false;
    bool debugMode = false;
    bool guiCommand = false;
    //QStringList opts;
    for (int i=0; i < argc; ++i) {
        char * val = argv[i];
        if(strcmp(val, "-s") == 0 || strcmp(val, "--server") == 0) {
            consoleMode = true;
        } else if(strcmp(val, "-b") == 0 || strcmp(val, "--verbose") == 0) {
            verboseMode = true;
        } else if(strcmp(val, "-d") == 0 || strcmp(val, "--debug") == 0) {
            debugMode = true;
        } else if(strcmp(val, "--reset-window") == 0) {
            guiCommand = true;
        }
    }


    // Must be called before XTEngine is initialized for the main app loop events.
    if(consoleMode || (argc > 1 && !verboseMode && !debugMode && !guiCommand)) {
        a = new QCoreApplication(argc, argv);
    } else {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        a = new QApplication(argc, argv);
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("XTPlayer");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption headlessOption(QStringList() << "s" << "server", "Run without GUI.");
    parser.addOption(headlessOption);
    QCommandLineOption importIniOption(QStringList() << "ii" << "importINI", "Import settings from an ini <file>", "file");
    parser.addOption(importIniOption);
    QCommandLineOption exportIniOption(QStringList() << "ei" << "exportINI", "Export settings to an ini <file>", "file");
    parser.addOption(exportIniOption);
    QCommandLineOption importJsonOption(QStringList() << "ij" << "importJSON", "Import settings from a JSON <file>", "file");
    parser.addOption(importJsonOption);
    QCommandLineOption exportJsonOption(QStringList() << "ej" << "exportJSON", "Export settings to a JSON <file>", "file");
    parser.addOption(exportJsonOption);

    QCommandLineOption addLibraryFolderOption(QStringList() << "a" << "add-media-folder", "Add media folder <folder>", "folder");
    parser.addOption(addLibraryFolderOption);
    QCommandLineOption excludeLibraryFolderOption(QStringList() << "exclude-media-folder", "Exclude media folder <folder>", "folder");
    parser.addOption(excludeLibraryFolderOption);
    QCommandLineOption removeLibraryFolderOption(QStringList() << "remove-media-folder", "Remove media or excluded folder <folder>", "folder");
    parser.addOption(removeLibraryFolderOption);
    QCommandLineOption listLibraryFolderOption(QStringList() << "list-media-folder", "List out media folders");
    parser.addOption(listLibraryFolderOption);
    QCommandLineOption thumbFolderOption(QStringList() << "thumb-folder", "Set the thumb folder <folder>", "folder");
    parser.addOption(thumbFolderOption);

    QCommandLineOption webServerPortOption(QStringList() << "web-server-port", "Set webserver port (1-65535) <number>", "number");
    parser.addOption(webServerPortOption);
    QCommandLineOption webSocketPortOption(QStringList() << "web-socket-port", "Set websocket port (1-65535 or -1 for auto) <number>", "number");
    parser.addOption(webSocketPortOption);
    QCommandLineOption webServerChunkSizeOption(QStringList() << "web-server-chunk-size", "Set webserver HLS chunk size in MB <number>", "number");
    parser.addOption(webServerChunkSizeOption);
    QCommandLineOption webImageCompressionOption(QStringList() << "web-thumb-compression", "Set thumb compression  (0-100 or -1 for uncompressed) <number>", "number");
    parser.addOption(webImageCompressionOption);
    QCommandLineOption webPassword(QStringList() << "web-password", "Set web server password <value>", "value");
    parser.addOption(webPassword);

    QCommandLineOption deviceComPortOption(QStringList() << "tcode-device-com-port", "Set tcode device com <value>", "value");
    parser.addOption(deviceComPortOption);
    QCommandLineOption deviceComOption(QStringList() << "tcode-device-com", "Set tcode device to com port");
    parser.addOption(deviceComOption);
    QCommandLineOption deviceNetworkOption(QStringList() << "tcode-device-network", "Set tcode device network");
    parser.addOption(deviceNetworkOption);

    QCommandLineOption deviceIPAddressOption(QStringList() << "tcode-device-address", "Set tcode device address (IP address or name) <value>", "value");
    parser.addOption(deviceIPAddressOption);
    QCommandLineOption deviceIPPortOption(QStringList() << "tcode-device-port", "Set tcode device port (1-65535) <number>", "number");
    parser.addOption(deviceIPPortOption);
    QCommandLineOption resetwindowOption(QStringList() << "reset-window", "Reset window size and position to default. (GUI mode only)");
    parser.addOption(resetwindowOption);
    QCommandLineOption resetOption(QStringList() << "reset", "Reset all settings to default");
    parser.addOption(resetOption);
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Start with debug output");
    parser.addOption(debugOption);
    QCommandLineOption verboseOption(QStringList() << "b" << "verbose", "Start with verbose output");
    parser.addOption(verboseOption);


    parser.process(*a);
    XTEngine* xtengine = new XTEngine();//Must be called before SettingsHandler functions are called.

    if(parser.isSet(verboseOption)) {
        LogHandler::Info("Starting in verbose mode");
        LogHandler::setUserDebug(true);
        LogHandler::setQtDebuging(true);
    } else if(parser.isSet(debugOption)) {
        LogHandler::Info("Starting in debug mode");
        LogHandler::setUserDebug(true);
    }
    if(parser.isSet(resetOption)) {
        LogHandler::Info("Resettings settings to default!");
        SettingsHandler::Default();
        LogHandler::Info("Settings cleared!");
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(addLibraryFolderOption)) {
        LogHandler::Info("Adding media folder..");
        QString targetDir = parser.value(addLibraryFolderOption);
        if(!targetDir.isEmpty() && QFileInfo::exists(targetDir)) {
            QStringList errors;
            if(SettingsHandler::addSelectedLibrary(targetDir, errors)) {
                SettingsHandler::Save();
                LogHandler::Info("Success!");
            } else
                LogHandler::Error(errors.join("\n"));
        } else {
            LogHandler::Error("Invalid directory: '"+ targetDir +"'");
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(webPassword)) {
        LogHandler::Info("Setting web password..");
        QString unencryptedPass = parser.value(webPassword);
        if(!unencryptedPass.isEmpty()) {
            if(!SettingsHandler::hashedWebPass().isEmpty()) {
                std::string var;
                LogHandler::Info("Current password: ");
                std::cin >> var;
                //fgets(var, SettingsHandler::hashedWebPass().length(), stdin);
                if(CryptHandler::checkPass(QString::fromStdString(var), SettingsHandler::hashedWebPass()) == PasswordResponse::CORRECT) {
                    std::string var;
                    LogHandler::Info("Confirm new password: ");
                    std::cin >> var;
                    //fgets(var, unencryptedPass.length(), stdin);
                    if(QString::fromStdString(var) == unencryptedPass) {
                        SettingsHandler::setHashedWebPass(CryptHandler::encryptPass(unencryptedPass));
                        SettingsHandler::Save();
                        LogHandler::Info("Success!");
                    } else {
                        LogHandler::Error("Password does not match");
                    }
                } else {
                    LogHandler::Error("Invalid password");
                }
            } else {
                std::string var;
                LogHandler::Info("Confirm password: ");
                std::cin >> var;
                //fgets(var, unencryptedPass.length(), stdin);
                if(QString::fromStdString(var) == unencryptedPass) {
                    SettingsHandler::setHashedWebPass(CryptHandler::encryptPass(unencryptedPass));
                    SettingsHandler::Save();
                    LogHandler::Info("Success!");
                } else {
                    LogHandler::Error("Password does not match");
                }
            }
        } else {
            LogHandler::Error("Invalid password");
        }
        delete xtengine;
        delete a;
        return 0;
    }



    if(parser.isSet(thumbFolderOption)) {
        LogHandler::Info("Setting thumb folder..");
        QString targetDir = parser.value(thumbFolderOption);
        if(!targetDir.isEmpty() && QFileInfo::exists(targetDir)) {
            SettingsHandler::setSelectedThumbsDir(targetDir);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        } else {
            LogHandler::Error("Invalid directory: '"+ targetDir +"'");
        }
        delete xtengine;
        delete a;
        return 0;
    }
    if(parser.isSet(excludeLibraryFolderOption)) {
        LogHandler::Info("Excluding folder..");
        QString targetDir = parser.value(excludeLibraryFolderOption);
        if(!targetDir.isEmpty() && QFileInfo::exists(targetDir)) {
            QStringList errors;
            if(SettingsHandler::addToLibraryExclusions(targetDir, errors)) {
                SettingsHandler::Save();
                LogHandler::Info("Success!");
            } else
                LogHandler::Error(errors.join("\n"));
        } else {
            LogHandler::Error("Invalid directory: '"+ targetDir +"'");
        }
        delete xtengine;
        delete a;
        return 0;
    }
    if(parser.isSet(removeLibraryFolderOption)) {
        QString targetDir = parser.value(removeLibraryFolderOption);
        if(!targetDir.isEmpty() && QFileInfo::exists(targetDir)) {
            QStringList folders = SettingsHandler::getSelectedLibrary();
            if(folders.contains(targetDir)) {
                LogHandler::Info("Removing library folder..");
                SettingsHandler::removeSelectedLibrary(targetDir);
                SettingsHandler::Save();
                LogHandler::Info("Success!");
                delete xtengine;
                delete a;
                return 0;
            }
            QStringList exclusions = SettingsHandler::getLibraryExclusions();
            if(exclusions.contains(targetDir)) {
                LogHandler::Info("Removing excluded folder..");
                SettingsHandler::removeFromLibraryExclusions(QList<int> {exclusions.indexOf(targetDir)});
                SettingsHandler::Save();
                LogHandler::Info("Success!");
                delete xtengine;
                delete a;
                return 0;
            }
            LogHandler::Info("Not found in libraries or excluded: "+targetDir);
            delete xtengine;
            delete a;
            return 0;
        } else {
            LogHandler::Error("Invalid directory: '"+ targetDir +"'");
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(importIniOption) || parser.isSet(importJsonOption) ) {
        QSettings::Format format = parser.isSet(importIniOption) ? QSettings::Format::IniFormat : JSONSettingsFormatter::JsonFormat;
        QString extension = format == QSettings::Format::IniFormat ? "ini" : "json";
        LogHandler::Info("Import settings from "+extension+" file");
        QString targetFile = format == QSettings::Format::IniFormat  ? parser.value(importIniOption) : parser.value(importJsonOption);
        if(!targetFile.endsWith(extension)) {
            LogHandler::Error("Invalid file: only "+extension+" files are valid: '"+ targetFile +"'");
            delete xtengine;
            delete a;
            return 1;
        }
        if(!XTPSettings::importFromFile(targetFile, format)) {
            delete xtengine;
            delete a;
            return 1;
        }
        LogHandler::Info("Success!");
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(exportIniOption) || parser.isSet(exportJsonOption)) {
        QSettings::Format format = parser.isSet(exportIniOption) ? QSettings::Format::IniFormat : JSONSettingsFormatter::JsonFormat;
        QString extension = format == QSettings::Format::IniFormat ? "ini" : "json";
        LogHandler::Info("Export settings to "+extension+" file");
        QString targetFile = format == QSettings::Format::IniFormat ? parser.value(exportIniOption) : parser.value(exportJsonOption);
        XTPSettings::exportToFile(targetFile, format);
        LogHandler::Info("Success!");
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(webServerChunkSizeOption)) {
        LogHandler::Info("Setting web server HLS chunk size");
        QString target = parser.value(webServerChunkSizeOption);
        bool ok;
        int i = target.toInt(&ok, 10);
        if(!ok) {
            LogHandler::Error("Invalid input: '"+ target +"'");
            delete xtengine;
            delete a;
            return 1;
        } else {
            SettingsHandler::setHTTPChunkSize(i * 1048576);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(webSocketPortOption)) {
        LogHandler::Info("Setting web socket port");
        QString targetPort = parser.value(webSocketPortOption);
        bool ok;
        int i = targetPort.toInt(&ok, 10);
        if(!ok) {
            LogHandler::Error("Invalid input: '"+ targetPort +"'");
            delete xtengine;
            delete a;
            return 1;
        } else {
            SettingsHandler::setWebSocketPort(i);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(webServerPortOption)) {
        LogHandler::Info("Setting web server port");
        QString targetPort = parser.value(webServerPortOption);
        bool ok;
        int i = targetPort.toInt(&ok, 10);
        if(!ok) {
            LogHandler::Error("Invalid input: '"+ targetPort +"'");
            delete xtengine;
            delete a;
            return 1;
        } else {
            SettingsHandler::setHTTPPort(i);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        }
        delete xtengine;
        delete a;
        return 0;
    }
    if(parser.isSet(webImageCompressionOption)) {
        LogHandler::Info("Setting web thumb compression");
        QString targetPort = parser.value(webImageCompressionOption);
        bool ok;
        int i = targetPort.toInt(&ok, 10);
        if(!ok) {
            LogHandler::Error("Invalid input: '"+ targetPort +"'");
            delete xtengine;
            delete a;
            return 1;
        } else {
            SettingsHandler::setHttpThumbQuality(i);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        }
        delete xtengine;
        delete a;
        return 0;
    }

//    QCommandLineOption deviceComOption(QStringList() << "tcode-device-com", "Set tcode device to com port");
//    QCommandLineOption deviceNetworkOption(QStringList() << "tcode-device-network", "Set tcode device network");

    if(parser.isSet(deviceComOption)) {
        LogHandler::Info("Setting tcode device com");
        SettingsHandler::setSelectedInputDevice(DeviceName::Serial);
        SettingsHandler::Save();
        LogHandler::Info("Success!");
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(deviceNetworkOption)) {
        LogHandler::Info("Setting tcode device network");
        SettingsHandler::setSelectedInputDevice(DeviceName::Network);
        SettingsHandler::Save();
        LogHandler::Info("Success!");
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(deviceComPortOption)) {
        LogHandler::Info("Setting tcode device com port");
        QString target = parser.value(deviceComPortOption);
        if(target.isEmpty()) {
            LogHandler::Error("Invalid input: '"+ target +"'");
            delete xtengine;
            delete a;
            return 1;
        } else {
            SettingsHandler::setSerialPort(target);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(deviceIPAddressOption)) {
        LogHandler::Info("Setting tcode device address");
        QString target = parser.value(deviceIPAddressOption);
        if(target.isEmpty()) {
            LogHandler::Error("Invalid input: '"+ target +"'");
            delete xtengine;
            delete a;
            return 1;
        } else {
            SettingsHandler::setServerAddress(target);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if(parser.isSet(deviceIPPortOption)) {
        LogHandler::Info("Setting tcode device port");
        QString targetPort = parser.value(deviceIPPortOption);
        bool ok;
        int i = targetPort.toInt(&ok, 10);
        if(!ok) {
            LogHandler::Error("Invalid input: '"+ targetPort +"'");
            delete xtengine;
            delete a;
            return 1;
        } else {
            SettingsHandler::setServerPort(targetPort);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        }
        delete xtengine;
        delete a;
        return 0;
    }
    if(parser.isSet(listLibraryFolderOption)) {
        QStringList folders = SettingsHandler::getSelectedLibrary();
        LogHandler::Info("Media folders:");
        if(folders.isEmpty()) {
            LogHandler::Info("None");
        }
        foreach (QString folder, folders) {
            LogHandler::Info(folder);
        }
        QStringList exclusions = SettingsHandler::getLibraryExclusions();
        LogHandler::Info("Excluded Media folders:");
        if(exclusions.isEmpty()) {
            LogHandler::Info("None");
        }
        foreach (QString folder, exclusions) {
            LogHandler::Info(folder);
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if (!consoleMode) {
        XTPSettings::load();
        if(parser.isSet("reset-window")) {
            LogHandler::Debug("Resettings window size to default!");
            XTPSettings::resetWindowSize();
        }

        MainWindow w(xtengine);
        w.show();
        int r = a->exec();
//        delete xtengine; // Make xtengine parent in Mainwindow
//        delete a; // Deleteing here caused crash in gui mode
        return r;
    }
    if(!SettingsHandler::getEnableHttpServer())
        SettingsHandler::setEnableHttpServer(true);
    xtengine->init();
    int r = a->exec();
    delete xtengine;
    delete a;
    return r;
}
