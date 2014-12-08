# Partner-space manager

Partner-space manager is a tool that can be used to modify the partner-space on Sailfish OS.
The partner-space is the left ambience panel, and can be replaced by any application that is 
*partner-enabled*.

*Partner-enabled* applications are simple Sailfish OS applications, where a specific flag
has been set on their main window. This flag allows the compositor to treat the partner-enabled
application in a different way, and store it on the left ambience panel.

## For developers

To write a partner-space, you need to provide the JSON metadata.
It should be installed inside `/usr/share/partnerspacemanager/partnerspaces/<partner-space-subfolder>`.

### The JSON metadata file

The metadata file contains information about the partner-space. It is a simple JSON file, that **must**
be named `partnerspace.json`.

This file contains the title of the partner-space, a quick description of the patch,
and other informations. Here is a sample of a metadata file.

```json
{
    "name": "My super partner-spaces",
    "description": "Some description.",
    "qml": "/usr/share/mypartnerspace/main.qml",
    "infos": {
        "maintainer": "Foo Bar"
    }
}

```

To launch the partner-space, you need to provide the application to be launched with 
partnerspace-manager. You can add either the `launcher` or `qml` field in the JSON metadata
file.

`launcher` requires the full path to the application to be launched, while `qml` requires an 
absolute path to the root QML file, for a QML-only application. If you don't provide these 
fields, partnerspace-manager will search for a file named `main.qml`, in the same folder as
the JSON metadata file, and use it as the root file.

`launcher` apps are responsible for setting the `windowProperty()` as follows:

    // These includes are needed.
    #include <QtGui/QGuiApplication>
    // Add the following to the .pro to alllow access to private headers
    // until this API makes it into a Qt release: QT += gui-private 
    #include <qpa/qplatformnativeinterface.h>
    
    // Add the create() call before accessing view->handle()
    view->create();
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    native->setWindowProperty(view->handle(), QLatin1String("CATEGORY"), QString(QLatin1String("partner")));
    
    // Just before pre-existing view->show()
    view->show();

#### Maintainers

A maintainer can be registered inside the JSON metadata file to claim maintainership of the patch. 
Either use your real name, as displayed on Github or Twitter, or use your usual nickname.
