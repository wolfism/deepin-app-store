/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <DApplication>
#include <DLog>
#include <QIcon>
#include <qcef_context.h>
#include <qcef_web_settings.h>
#include <DGuiApplicationHelper>
#include <DPlatformWindowHandle>
#include <DApplicationSettings>

#include "base/consts.h"
#include "resources/images.h"
#include "resources/theme.h"
#include "services/dbus_manager.h"
#include "services/settings_manager.h"
#include "services/rcc_scheme_handler.h"
#include "ui/web_window.h"

namespace
{

const char kEnableDomStorageFlush[] = "--enable-aggressive-domstorage-flushing";

const char kDisableGpu[] = "--disable-gpu";

const char kEnableLogging[] = "--enable-logging";

const char kLogLevel[] = "--log-level";

}  // namespace

int main(int argc, char **argv)
{
    qputenv("DXCB_FAKE_PLATFORM_NAME_XCB", "true");

    QCefGlobalSettings settings;
    // Do not use sandbox.
    settings.setNoSandbox(true);

    if (qEnvironmentVariableIntValue("QCEF_DEBUG") == 1) {
        // Open http://localhost:9222 in chromium browser to see dev tools.
        settings.setRemoteDebug(true);
        settings.setLogSeverity(QCefGlobalSettings::LogSeverity::Verbose);
    }
    else {
        settings.setRemoteDebug(false);
        settings.setLogSeverity(QCefGlobalSettings::LogSeverity::Error);
    }

    // Disable GPU process.
    settings.addCommandLineSwitch(kDisableGpu, "");

    // Enable aggressive storage commit to minimize data loss.
    // See public/common/content_switches.cc.
    settings.addCommandLineSwitch(kEnableDomStorageFlush, "");

    // Set web cache folder.
    QDir cache_dir(dstore::GetCacheDir());
    cache_dir.mkpath(".");
    settings.setCachePath(cache_dir.filePath("cache"));
    settings.setUserDataPath(cache_dir.filePath("cef-storage"));

    // TODO(Shaohua): Rotate console log.
    settings.setLogFile(cache_dir.filePath("web-console.log"));
    settings.addCommandLineSwitch(kEnableLogging, "");
    settings.addCommandLineSwitch(kLogLevel, "0");
    settings.addCommandLineSwitch("--use-views", "");

    auto themName = dstore::SettingsManager::instance()->appThemeName();
    if (themName.isEmpty()) {
        themName = "light";
        dstore::SettingsManager::instance()->setThemeName(themName);
    }
    settings.setCustomSchemeHandler(dstore::RccSchemeHandler);
    settings.addCustomScheme(QUrl("rcc://web"));
    settings.setBackgroundColor(dstore::BackgroundColor(themName));

    if (QCefInit(argc, argv, settings) >= 0) {
        return 0;
    }

#ifndef DSTORE_NO_DXCB
    Dtk::Widget::DApplication::loadDXcbPlugin();
#endif

    Dtk::Widget::DApplication app(argc, argv);
    if (!Dtk::Widget::DPlatformWindowHandle::pluginVersion().isEmpty()) {
        app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    }

    app.setAttribute(Qt::AA_EnableHighDpiScaling, true);
    app.setWindowIcon(QIcon(dstore::kImageDeepinAppStore));
    app.setProductIcon(QIcon(dstore::kImageDeepinAppStore));
    app.setOrganizationName("deepin");
    app.setOrganizationDomain("deepin.com");
    app.setApplicationVersion(Dtk::Widget::DApplication::buildVersion("5.6.0.0"));
    app.setApplicationName(dstore::kAppName);
    app.loadTranslator();
    app.setApplicationDisplayName(QObject::tr("App Store"));
    app.setApplicationDescription(QObject::tr(
        "App Store is a software center with diverse and quality applications, supporting installation and uninstallation with one click."
        ));
    app.setApplicationAcknowledgementPage(
        "https://www.deepin.org/acknowledgments/deepin-app-store/");

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    if (!DGuiApplicationHelper::setSingleInstance("com.deepin.AppStore")) {
        qWarning() << "another store is running";
        return 0;
    }
    app.setAutoActivateWindows(true);

    // fix error for cutelogger
    // No appenders associated with category js
    auto category = "js";
    auto fileAppender = new Dtk::Core::RollingFileAppender(Dtk::Core::DLogManager::getlogFilePath());
    static Dtk::Core::Logger customLoggerInstance(category);
    customLoggerInstance.logToGlobalInstance(category, true);
    customLoggerInstance.registerAppender(fileAppender);

    dstore::DBusManager dbus_manager;
    if (!dbus_manager.registerDBus()) {
        // register failed, another store running
        // Exit process after 1000ms.
        QTimer::singleShot(1000, [&]()
        {
            app.quit();
        });
        return app.exec();
    }


    QCefBindApp(&app);

    Dtk::Widget::DApplicationSettings savetheme;

    dstore::WebWindow window;

    window.setupDaemon(&dbus_manager);

    app.installEventFilter(&window);

    window.setQCefSettings(&settings);
    window.loadPage();
    window.showWindow();

    return app.exec();
}
