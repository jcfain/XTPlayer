#include "mainwindow.h"

#include <QApplication>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{

    qRegisterMetaType<QItemSelection>();
    qRegisterMetaTypeStreamOperators<QList<QString>>("QList<QString>");
    qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
    qRegisterMetaType<AxisDimension>("AxisDimension");
    qRegisterMetaType<AxisType>("AxisType");
    qRegisterMetaTypeStreamOperators<AxisName>("AxisName");
    qRegisterMetaTypeStreamOperators<DecoderModel>("DecoderModel");
    qRegisterMetaTypeStreamOperators<XMediaStatus>("XMediaStatus");
    qRegisterMetaType<LibraryListItem>();
    qRegisterMetaTypeStreamOperators<LibraryListItem>("LibraryListItem");
    qRegisterMetaTypeStreamOperators<QMap<QString, QList<LibraryListItem>>>("QMap<QString, QList<LibraryListItem>>");
    qRegisterMetaTypeStreamOperators<QList<LibraryListItem>>("QList<LibraryListItem>");
    qRegisterMetaType<LibraryListItem27>();
    qRegisterMetaTypeStreamOperators<LibraryListItem27>("LibraryListItem27");
    qRegisterMetaTypeStreamOperators<QMap<QString, QList<LibraryListItem27>>>("QMap<QString, QList<LibraryListItem27>>");
    qRegisterMetaTypeStreamOperators<QList<LibraryListItem27>>("QList<LibraryListItem27>");
    qRegisterMetaTypeStreamOperators<TCodeVersion>("TCodeVersion");
    qRegisterMetaTypeStreamOperators<LibraryListItemMetaData>("LibraryListItemMetaData");
    qRegisterMetaTypeStreamOperators<LibraryListItemMetaData258>("LibraryListItemMetaData258");
    qRegisterMetaTypeStreamOperators<Bookmark>("Bookmark");
    qRegisterMetaType<QVector<int> >("QVector<int>");

    QCommandLineParser parser;
    parser.setApplicationDescription("XTPlayer");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption headlessOption(QStringList() << "s" << "server", "Run without GUI.");
    parser.addOption(headlessOption);
    QCommandLineOption addLibraryFolderOption(QStringList() << "a" << "add-media-folder", "Add media folder <folder>", "folder");
    parser.addOption(addLibraryFolderOption);
    QCommandLineOption importOption(QStringList() << "i" << "import", "Import settings from an ini <file>", "file");
    parser.addOption(importOption);
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Start with debug output");
    parser.addOption(debugOption);
    QCommandLineOption verboseOption(QStringList() << "b" << "verbose", "Start with verbose output");
    parser.addOption(verboseOption);
    QCommandLineOption resetOption(QStringList() << "reset", "Reset all settings to default");
    parser.addOption(resetOption);
    QCommandLineOption resetwindowOption(QStringList() << "reset-window", "Reset window size and position to default. (GUI mode only)");
    parser.addOption(resetwindowOption);
    QCommandLineOption webServerPortOption(QStringList() << "web-server-port", "Set webserver port (1-65535) <number>", "number");
    parser.addOption(webServerPortOption);
    QCommandLineOption deviceIPAddressOption(QStringList() << "tcode-device-address", "Set tcode device address (IP address or name) <value>", "value");
    parser.addOption(deviceIPAddressOption);
    QCommandLineOption deviceIPPortOption(QStringList() << "tcode-device-port", "Set tcode device port (1-65535) <number>", "number");
    parser.addOption(deviceIPPortOption);


    QCoreApplication *a = new QCoreApplication(argc, argv);
    parser.process(*a);
    bool isGUI = parser.isSet(headlessOption);

    XTEngine* xtengine = new XTEngine();//Must be called before SettingsHandler functions are called.

    if(parser.isSet(verboseOption)) {
        LogHandler::Info("Starting in verbose mode");
        LogHandler::setUserDebug(true);
        LogHandler::setQtDebuging(true);
    }
    if(parser.isSet(debugOption)) {
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
        LogHandler::Info("Adding library..");
        QString targetDir = parser.value(addLibraryFolderOption);
        if(!targetDir.isEmpty() && QFileInfo::exists(targetDir)) {
            SettingsHandler::addSelectedLibrary(targetDir);
            SettingsHandler::Save();
            LogHandler::Info("Success!");
        } else {
            LogHandler::Error("Invalid directory: '"+ targetDir +"'");
        }
        delete xtengine;
        delete a;
        return 0;
    }
    if(parser.isSet(importOption)) {
        LogHandler::Info("Import settings from file");
        QString targetFile = parser.value(importOption);
        if(!targetFile.endsWith("ini")) {
            LogHandler::Error("Invalid file: only ini files are valid: '"+ targetFile +"'");
            delete xtengine;
            delete a;
            return 1;
        }
        if(SettingsHandler::Import(targetFile)) {
            LogHandler::Info("Success!");
            delete xtengine;
            delete a;
            return 0;
        }
        LogHandler::Info("Error importing file: '"+ targetFile +"'");
        delete xtengine;
        delete a;
        return 1;
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
        }
        delete xtengine;
        delete a;
        return 0;
    }

    if (!isGUI) {
        delete a;
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        a = new QApplication(argc, argv);
        #ifdef _WIN32
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
        }
        #endif

        XTPSettings::load();
        if(parser.isSet("reset-window")) {
            LogHandler::Debug("Resettings window size to default!");
            XTPSettings::resetWindowSize();
        }

        MainWindow w(xtengine);
        w.show();
        int r = a->exec();
        delete xtengine;
        delete a;
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
