#include "plugin.h"
#include "pluginmanager.h"

#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>

namespace OpenCOR {

Plugin::Plugin(const QString &pFileName,
               const PluginInfo::PluginType &pGuiOrConsoleType) :
    mName(name(pFileName)),
    // Note: to get the name of the plugin from its file name, we must remove
    //       the plugin prefix part from it...
    mInfo(),
    mInstance(0),
    mStatus(Undefined)
{
    // Check whether the plugin physically exists

    if (QFileInfo(pFileName).exists()) {
        // The plugin exists, so retrieve its information

        mInfo = info(pFileName);

        // Try to load the plugin, but only if it is either a general plugin or
        // one of the type we are happy with

        if (   (mInfo.mType == PluginInfo::General)
            || (mInfo.mType == pGuiOrConsoleType)) {
            // We are dealing with the right kind of plugin, so try to load it
//---GRY--- WE SHOULD CHECK IN THE SETTINGS WHETHER THE USER ACTUALLY WANTs TO
//          LOAD THE PLUGIN OR NOT...

            QPluginLoader pluginLoader(pFileName);

            if (pluginLoader.load()) {
                // The plugin has been properly loaded, so...

                mInstance = pluginLoader.instance();

                mStatus = Loaded;
            } else {
                // The plugin couldn't be loaded for some reason (surely, this
                // should never happen...?!), so...

                mStatus = NotLoaded;
            }
        } else if (mInfo.mType == PluginInfo::Undefined) {
            // We couldn't retrieve the plugin information which means we are
            // not dealing with an OpenCOR plugin or that one or several of the
            // plugin's dependencies weren't loaded, so...

            mStatus = NotPluginOrMissingDependencies;
        } else {
            // We are dealing with a plugin that is not of the type we are happy
            // with (i.e. it's a console plugin but we are running the GUI
            // version of OpenCOR, or it's a GUI plugin but we are running the
            // console version of OpenCOR), so...

            mStatus = NotSuitable;
        }
    } else {
        // The plugin doesn't exist, so...

        mStatus = NotFound;
    }
}

QString Plugin::name()
{
    // Return the plugin's name

    return mName;
}

PluginInfo Plugin::info()
{
    // Return the plugin's information

    return mInfo;
}

QObject * Plugin::instance()
{
    // Return the plugin's insance

    return mInstance;
}

Plugin::PluginStatus Plugin::status()
{
    // Return the plugin's status

    return mStatus;
}

QString Plugin::name(const QString &pFileName)
{
    // Return the plugin's name based on its file name

    return QFileInfo(pFileName).baseName().remove(0, PluginPrefix.length());
    // Note: we must remove the plugin prefix part from the plugin file name...
}

QString Plugin::fileName(const QString &pDir, const QString &pName)
{
    // Return the plugin's file name based on its name

    return pDir+QDir::separator()+PluginPrefix+pName+PluginExtension;
    // Note: we must add the plugin prefix part to the plugin file name...
}

PluginInfo Plugin::info(const QString &pFileName)
{
    // Return the plugin's information
    // Note: in order to do this, we must, on Windows, keep track of the current
    //       directory and go to the directory where our plugin is, thus making
    //       sure that we can retrieve the plugin's information without any
    //       problem. Indeed, if we think of the CellMLModelRepository plugin,
    //       it requires the Core plugin to be loaded even if all we want is to
    //       retrieve its information, so...

    typedef PluginInfo (*PluginInfoFunc)();

#ifdef Q_WS_WIN
    QString origPath = QDir::currentPath();

    QDir::setCurrent(QFileInfo(pFileName).absolutePath());
#endif

    PluginInfoFunc pluginInfoFunc = (PluginInfoFunc) QLibrary::resolve(pFileName,
                                                                       QString(name(pFileName)+"PluginInfo").toLatin1().constData());

#ifdef Q_WS_WIN
    QDir::setCurrent(origPath);
#endif

    // Check whether the plugin information function was found

    if (pluginInfoFunc)
        // The plugin information function was found, so we can extract the
        // information we are after

        return pluginInfoFunc();
    else
        return PluginInfo();
}

}
