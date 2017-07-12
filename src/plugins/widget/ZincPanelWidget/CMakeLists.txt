PROJECT(ZincPanelWidgetPlugin)

# Add the plugin

ADD_PLUGIN(ZincPanelWidget
    SOURCES
        ../../i18ninterface.cpp
        ../../plugininfo.cpp

        src/zincpanelswidget.cpp
        src/zincpanelwidget.cpp
        src/zincpanelwidgetplugin.cpp
    HEADERS_MOC
        src/zincpanelswidget.h
        src/zincpanelwidget.h
        src/zincpanelwidgetplugin.h
    PLUGINS
        Core
        Zinc
)
