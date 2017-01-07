/*
 * Copyright (C) 2016-2017  Luca Weiss <luca (at) z3ntu (dot) xyz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>
#include <QDomDocument>
//TODO Remove QDBusArgument include
#include <QDBusArgument>

#include <iostream>

#include "razermethods.h"

namespace razermethods
{
/**
 * Gets a list of connected devices in form of their serial number.
 * Can be used to create a 'Device' object and get further information about the devices.
 */
QStringList getConnectedDevices()
{
    QDBusMessage m = prepareGeneralQDBusMessage("razer.devices", "getDevices");
    return QDBusMessageToStringList(m);
}

/**
 * If devices should sync effects.
 * Example: Set it to 'on', set the lighting on one device to something, other devices connected will automatically get set to the same effect.
 */
bool syncDevices(bool yes)
{
    QDBusMessage m = prepareGeneralQDBusMessage("razer.devices", "syncEffects");

    // Set arguments
    QList<QVariant> args;
    //yes ? args.append("True") : args.append("False"); // maybe bool works here
    args.append(yes);
    m.setArguments(args);

    bool queued = QDBusConnection::sessionBus().send(m);
    std::cout << "Queued: " << queued << std::endl;
    return queued;
}

/**
 * Returns the daemon version currently running.
 */
QString getDaemonVersion()
{
    QDBusMessage m = prepareGeneralQDBusMessage("razer.daemon", "version");
    return QDBusMessageToString(m);
}

/**
 * Stops the daemon. WARNING: FURTHER COMMUNICATION WILL NOT BE POSSIBLE.
 */
bool stopDaemon()
{
    QDBusMessage m = prepareGeneralQDBusMessage("razer.daemon", "stop");
    return QDBusMessageToVoid(m);
}

/**
 * Sets if the Chroma lighting should turn off if the screensaver is turned on.
 */
bool setTurnOnScreensaver(bool turnOffOnScreensaver)
{
    QDBusMessage m;
    m = prepareGeneralQDBusMessage("razer.devices", "enableTurnOffOnScreensaver");
    QList<QVariant> args;
    //yes ? args.append("True") : args.append("False"); // maybe bool works here
    args.append(turnOffOnScreensaver);
    m.setArguments(args);

    return QDBusMessageToVoid(m);
}

/* Device class methods */
/**
 * Constrcuts a new device object with the given serial.
 */
Device::Device(QString s)
{
    serial = s;
    Introspect();
    setupCapabilities();
}

/**
 * Returns a human readable device name like "Razer DeathAdder Chroma" or "Razer Kraken 7.1 (Rainie)".
 */
QString Device::getDeviceName()
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.misc", "getDeviceName");
    return QDBusMessageToString(m);
}

/**
 * Returns the type of the device. Could be one of 'headset', 'mouse', 'mug', 'keyboard', 'tartarus' or another type, if added to the daemon.
 */
QString Device::getDeviceType()
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.misc", "getDeviceType");
    return QDBusMessageToString(m);
}

/**
 * Returns the firmware version of the device.
 */
QString Device::getFirmwareVersion()
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.misc", "getFirmware");
    return QDBusMessageToString(m);
}

/**
 * Returns the .png filename for pictures.
 * Could return an empty string (qstring.isEmpty()) if no picture was found.
 */
QString Device::getPngFilename()
{
    if(!urlLookup.value(getPid()).isEmpty())
        return urlLookup.value(getPid()).split("/").takeLast();
    return QString();
}

/**
 * Returns the download url for pictures.
 * Could return an empty string (qstring.isEmpty()) if no picture was found.
 */
QString Device::getPngUrl()
{
//     if(!urlLookup_dev.value(getPid()).isEmpty())
//         return "http://developer.razerzone.com/wp-content/uploads/" + urlLookup_dev.value(getPid()) + ".png";
    /*else*/ if(!urlLookup.value(getPid()).isEmpty())
        return "http://assets.razerzone.com/eeimages/products/" + urlLookup.value(getPid());
    return QString();
}

/**
 * Sets the logo to static lighting.
 */
bool Device::setLogoStatic(int r, int g, int b)
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.lighting.logo", "setLogoStatic");
    QList<QVariant> args;
    args.append(r);
    args.append(g);
    args.append(b);
    m.setArguments(args);
    return QDBusMessageToVoid(m);
}

/**
 * Sets the brightness.
 */
bool Device::setBrightness(double brightness)
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.lighting.brightness", "setBrightness"); //TODO Check
    QList<QVariant> args;
    args.append(brightness);
    m.setArguments(args);
    return QDBusMessageToVoid(m);
}

/**
 * Sets the logo brightness.
 */
bool Device::setLogoBrightness(double brightness)
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.lighting.logo", "setLogoBrightness");
    QList<QVariant> args;
    args.append(brightness);
    m.setArguments(args);
    return QDBusMessageToVoid(m);
}

/**
 * Sets the scroll wheel brightness.
 */
bool Device::setScrollBrightness(double brightness)
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.lighting.scroll", "setScrollBrightness");
    QList<QVariant> args;
    args.append(brightness);
    m.setArguments(args);
    return QDBusMessageToVoid(m);
}

/**
 * Return usb vendor id as integer in decimal notation. Should always be 5426 (-> Hex 1532)
 */
int Device::getVid()
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.misc", "getVidPid");
    return QDBusMessageToIntArray(m)[0];
}

/**
 * Return usb product id as integer in decimal notation.
 */
int Device::getPid()
{
    QDBusMessage m = prepareDeviceQDBusMessage("razer.device.misc", "getVidPid");
    return QDBusMessageToIntArray(m)[1];
}

/**
 * Fill "introspection" variable with data from the dbus introspection xml
 */
void Device::Introspect()
{
    QStringList intr;

    QDBusMessage m = prepareDeviceQDBusMessage("org.freedesktop.DBus.Introspectable", "Introspect");
    QDomDocument doc = QDBusMessageToXML(m);

    QDomNodeList nodes = doc.documentElement().childNodes();
    for(int i = 0; i<nodes.count(); i++) {
        // Check if "interface" and also not org.freedesktop.DBus.Introspectable
        QDomElement element = nodes.at(i).toElement();
        QString interfacename = element.attributeNode("name").value();

        QDomNodeList methodnodes = element.childNodes();
        for(int ii = 0; ii<methodnodes.count(); ii++) {
            QDomElement methodelement = methodnodes.at(ii).toElement();
            intr.append(interfacename + ";" + methodelement.attributeNode("name").value());
        }
        intr.append(interfacename);
    }
    introspection = intr;
}

/**
 * Fill "capabilities" list with the capabilities of the device. Names are from the pylib, parsed with the script ./capabilities_to_cpp.sh in the root of this repo.
 */
void Device::setupCapabilities()
{
    capabilities.insert("name", true);
    capabilities.insert("type", true);
    capabilities.insert("firmware_version", true);
    capabilities.insert("serial", true);
    capabilities.insert("brightness", hasCapabilityInternal("razer.device.lighting.brightness"));

    capabilities.insert("macro_logic", hasCapabilityInternal("razer.device.macro"));

    // Default device is a chroma so lighting capabilities
    capabilities.insert("lighting", hasCapabilityInternal("razer.device.lighting.chroma"));
    capabilities.insert("lighting_breath_single", hasCapabilityInternal("razer.device.lighting.chroma", "setBreathSingle"));
    capabilities.insert("lighting_breath_dual", hasCapabilityInternal("razer.device.lighting.chroma", "setBreathDual"));
    capabilities.insert("lighting_breath_triple", hasCapabilityInternal("razer.device.lighting.chroma", "setBreathTriple"));
    capabilities.insert("lighting_breath_random", hasCapabilityInternal("razer.device.lighting.chroma", "setBreathRandom"));
    capabilities.insert("lighting_wave", hasCapabilityInternal("razer.device.lighting.chroma", "setWave"));
    capabilities.insert("lighting_reactive", hasCapabilityInternal("razer.device.lighting.chroma", "setReactive"));
    capabilities.insert("lighting_none", hasCapabilityInternal("razer.device.lighting.chroma", "setNone"));
    capabilities.insert("lighting_spectrum", hasCapabilityInternal("razer.device.lighting.chroma", "setSpectrum"));
    capabilities.insert("lighting_static", hasCapabilityInternal("razer.device.lighting.chroma", "setStatic"));
    capabilities.insert("lighting_ripple", hasCapabilityInternal("razer.device.lighting.custom", "setRipple")); // Thinking of extending custom to do more hence the key check
    capabilities.insert("lighting_ripple_random", hasCapabilityInternal("razer.device.lighting.custom", "setRippleRandomColour"));

    capabilities.insert("lighting_pulsate", hasCapabilityInternal("razer.device.lighting.chroma", "setPulsate"));

    // Get if the device has an LED Matrix, == True as its a DBus boolean otherwise, so for consistency sake we coerce it into a native bool
    // FIXME: 'lighting_led_matrix': self._dbus_interfaces['device'].hasMatrix() == True,
    capabilities.insert("lighting_led_single", hasCapabilityInternal("razer.device.lighting.chroma", "setKey"));

    // Mouse lighting attrs
    capabilities.insert("lighting_logo", hasCapabilityInternal("razer.device.lighting.logo"));
    capabilities.insert("lighting_logo_active", hasCapabilityInternal("razer.device.lighting.logo", "setLogoActive"));
    capabilities.insert("lighting_logo_blinking", hasCapabilityInternal("razer.device.lighting.logo", "setLogoBlinking"));
    capabilities.insert("lighting_logo_brightness", hasCapabilityInternal("razer.device.lighting.logo", "setLogoBrightness"));
    capabilities.insert("lighting_logo_pulsate", hasCapabilityInternal("razer.device.lighting.logo", "setLogoPulsate"));
    capabilities.insert("lighting_logo_spectrum", hasCapabilityInternal("razer.device.lighting.logo", "setLogoSpectrum"));
    capabilities.insert("lighting_logo_static", hasCapabilityInternal("razer.device.lighting.logo", "setLogoStatic"));
    capabilities.insert("lighting_logo_none", hasCapabilityInternal("razer.device.lighting.logo", "setLogoNone"));
    capabilities.insert("lighting_logo_reactive", hasCapabilityInternal("razer.device.lighting.logo", "setLogoReactive"));
    capabilities.insert("lighting_logo_breath_single", hasCapabilityInternal("razer.device.lighting.logo", "setLogoBreathSingle"));
    capabilities.insert("lighting_logo_breath_dual", hasCapabilityInternal("razer.device.lighting.logo", "setLogoBreathDual"));
    capabilities.insert("lighting_logo_breath_random", hasCapabilityInternal("razer.device.lighting.logo", "setLogoBreathRandom"));

    capabilities.insert("lighting_scroll", hasCapabilityInternal("razer.device.lighting.scroll"));
    capabilities.insert("lighting_scroll_active", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollActive"));
    capabilities.insert("lighting_scroll_blinking", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollBlinking"));
    capabilities.insert("lighting_scroll_brightness", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollBrightness"));
    capabilities.insert("lighting_scroll_pulsate", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollPulsate"));
    capabilities.insert("lighting_scroll_spectrum", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollSpectrum"));
    capabilities.insert("lighting_scroll_static", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollStatic"));
    capabilities.insert("lighting_scroll_none", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollNone"));
    capabilities.insert("lighting_scroll_reactive", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollReactive"));
    capabilities.insert("lighting_scroll_breath_single", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollBreathSingle"));
    capabilities.insert("lighting_scroll_breath_dual", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollBreathDual"));
    capabilities.insert("lighting_scroll_breath_random", hasCapabilityInternal("razer.device.lighting.scroll", "setScrollBreathRandom"));
}

/**
 * Returns if a device has a given capability.
 */
bool Device::hasCapability(const QString &name)
{
    return capabilities.value(name);
}

/**
 * Internal method to determine whether a device has a given capability based on interface and method names.
 */
bool Device::hasCapabilityInternal(const QString &interface, const QString &method)
{
    if(method.isNull()) {
        return introspection.contains(interface);
    }
    return introspection.contains(interface + ";" + method);
}

/**
 * Returns a QHash object with all capabilities.
 */
QHash<QString, bool> Device::getAllCapabilities()
{
    return capabilities;
}

//TODO Signal for deviceAdded and deviceRemoved
// QDBusConnection::sessionBus().connect("org.gnome.SessionManager", "/org/gnome/SessionManager/Presence", "org.gnome.SessionManager.Presence" ,"StatusChanged", this, SLOT(MySlot(uint)));

/**
 * Destructor
 */
Device::~Device()
{
}

/**
 * Returns a QDBusMessage object for the given device ("org/razer/serial").
 */
QDBusMessage Device::prepareDeviceQDBusMessage(const QString &interface, const QString &method)
{
    return QDBusMessage::createMethodCall("org.razer", "/org/razer/device/" + serial, interface, method);
}

/**
 * Returns a QDBusMessage object for general daemon use ("/org/razer").
 */
QDBusMessage prepareGeneralQDBusMessage(const QString &interface, const QString &method)
{
    return QDBusMessage::createMethodCall("org.razer", "/org/razer", interface, method);
}

/**
 * Sends a QDBusMessage and returns the string value.
 */
QString QDBusMessageToString(const QDBusMessage &message)
{
    QDBusMessage msg = QDBusConnection::sessionBus().call(message);
    if(msg.type() == QDBusMessage::ReplyMessage) {
        // Everything went fine.
        return msg.arguments()[0].toString();
    }
    // TODO: Handle error
    return "error";
}

/**
 * Sends a QDBusMessage and returns the stringlist value.
 */
QStringList QDBusMessageToStringList(const QDBusMessage &message)
{
    QDBusMessage msg = QDBusConnection::sessionBus().call(message);
    if(msg.type() == QDBusMessage::ReplyMessage) {
        return msg.arguments()[0].toStringList();
    }
    // TODO: Handle errror
    return msg.arguments()[0].toStringList();
}

/**
 * Sends a QDBusMessage and returns the int array value.
 */
//QList<int>
QList<int> QDBusMessageToIntArray(const QDBusMessage &message)
{
    QList<int> *retList = new QList<int>();
    QDBusMessage msg = QDBusConnection::sessionBus().call(message);
    if(msg.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "reply :" << msg; // sth like QDBusMessage(type=MethodReturn, service=":1.1482", signature="ai", contents=([Argument: ai {5426, 67}]) )
        qDebug() << "reply arguments : " << msg.arguments();
        qDebug() << "reply[0] :" << msg.arguments().at(0);
        //ugh
        const QDBusArgument myArg = msg.arguments().at(0).value<QDBusArgument>();
        myArg.beginArray();
        while (!myArg.atEnd()) {
            int myElement = qdbus_cast<int>(myArg);
//             std::cout << myElement << std::endl;
            retList->append(myElement);
        }
        myArg.endArray();
    }
    // TODO: Handle errror
    //return msg.arguments()[0].toList();
    return *retList;
}

/**
 * Sends a QDBusMessage and returns the xml value.
 */
QDomDocument QDBusMessageToXML(const QDBusMessage &message)
{
    QDBusMessage msg = QDBusConnection::sessionBus().call(message);
    QDomDocument doc;
    if(msg.type() == QDBusMessage::ReplyMessage) {
        doc.setContent(msg.arguments()[0].toString());
    }
    // TODO: Handle errror
    return doc;
}

/**
 * Sends a QDBusMessage and returns if the call was successful.
 */
bool QDBusMessageToVoid(const QDBusMessage &message)
{
    return QDBusConnection::sessionBus().send(message);
    // TODO: Handle errror
}
}

// Main method for testing / playing.
int main()
{
    std::cout << "Daemon version: " << razermethods::getDaemonVersion().toStdString() << std::endl;
    QStringList serialnrs = razermethods::getConnectedDevices();
    razermethods::syncDevices(false);
    foreach (const QString &str, serialnrs) {
        std::cout << "-----------------" << std::endl;
//         std::cout << "Serial: " << str.toStdString() << std::endl;
        razermethods::Device device = razermethods::Device(str);
        //device.setLogoStatic(0, 255, 0);
        //device.getVid();
        //device.getPid();
//         std::cout << "Name:   " << device.getDeviceName().toStdString() << std::endl;
//         std::cout << "Type:   " << device.getDeviceType().toStdString() << std::endl;
//         //bool logostatic = device.hasCapabilityInternal("razer.device.lighting.logo", "setLogoStatic");
//         //std::cout << "Can logo static: " << logostatic << std::endl;
//         QHash<QString, bool> hash = device.getAllCapabilities();
//         for (QHash<QString, bool>::iterator i = hash.begin(); i != hash.end(); ++i)
//             std::cout << i.key().toStdString() << ": " << i.value() << std::endl;
    }
}
