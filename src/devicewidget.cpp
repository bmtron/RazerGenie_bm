/*
 * Copyright (C) 2017-2018  Luca Weiss <luca (at) z3ntu (dot) xyz>
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

#include "devicewidget.h"

#include "customeditor/customeditor.h"
#include "dpisliderwidget.h"
#include "ledwidget.h"
#include "util.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTextEdit>
#include <QVBoxLayout>

DeviceWidget::DeviceWidget(const QString &name, const QDBusObjectPath &devicePath, libopenrazer::Device *device)
    : QWidget()
{
    this->name = name;
    this->devicePath = devicePath;
    this->device = device;

    auto *verticalLayout = new QVBoxLayout(this);

    // List of locations to iterate through
    QList<libopenrazer::Led *> leds = device->getLeds();

    // Declare header font
    QFont headerFont("Arial", 15, QFont::Bold);
    QFont titleFont("Arial", 18, QFont::Bold);

    // Add header with the device name
    QLabel *header = new QLabel(name, this);
    header->setFont(titleFont);
    verticalLayout->addWidget(header);

    // Lighting header
    if (leds.size() != 0) {
        QLabel *lightingHeader = new QLabel(tr("Lighting"), this);
        lightingHeader->setFont(headerFont);
        verticalLayout->addWidget(lightingHeader);
    }

    // Iterate through lighting locations
    for (libopenrazer::Led *led : leds) {
        verticalLayout->addWidget(new LedWidget(this, device, led));
    }

    /* DPI sliders */
    if (device->hasFeature("dpi")) {
        if (device->hasFeature("restricted_dpi")) {
            // FIXME
            // verticalLayout->addWidget(new DPIComboWidget(this, device));
        } else {
            verticalLayout->addWidget(new DpiSliderWidget(this, device));
        }
    }

    /* Poll rate */
    if (device->hasFeature("poll_rate")) {
        QLabel *pollRateHeader = new QLabel(tr("Polling rate"), this);
        pollRateHeader->setFont(headerFont);
        verticalLayout->addWidget(pollRateHeader);

        ushort pollRate = 0;
        try {
            pollRate = device->getPollRate();
        } catch (const libopenrazer::DBusException &e) {
            qWarning("Failed to get poll rate");
        }

        QVector<ushort> supportedPollRates;
        try {
            supportedPollRates = device->getSupportedPollRates();
        } catch (const libopenrazer::DBusException &e) {
            qWarning("Failed to get supported poll rates");
        }

        auto *pollComboBox = new QComboBox;
        for (ushort poll : supportedPollRates) {
            pollComboBox->addItem(QString::number(poll) + " Hz", poll);
        }
        pollComboBox->setCurrentText(QString::number(pollRate) + " Hz");
        verticalLayout->addWidget(pollComboBox);

        connect(pollComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DeviceWidget::pollCombo);
    }

    /* Custom lighting */
    if (device->hasFeature("custom_frame")) {
        auto *button = new QPushButton(this);
        button->setText(tr("Open custom editor"));
        verticalLayout->addWidget(button);
        connect(button, &QPushButton::clicked, this, &DeviceWidget::openCustomEditor);
#ifdef INCLUDE_MATRIX_DISCOVERY
        QPushButton *buttonD = new QPushButton(this);
        buttonD->setText(tr("Launch matrix discovery"));
        verticalLayout->addWidget(buttonD);
        connect(buttonD, &QPushButton::clicked, this, [this] { openCustomEditor(true); });
#endif
    }

    /* Spacer to bottom */
    auto *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(spacer);

    /* Serial and firmware version labels */
    QString serial = "error";
    try {
        serial = device->getSerial();
    } catch (const libopenrazer::DBusException &e) {
        qWarning("Failed to get serial");
    }
    QString firmwareVersion = "error";
    try {
        firmwareVersion = device->getFirmwareVersion();
    } catch (const libopenrazer::DBusException &e) {
        qWarning("Failed to get firmware version");
    }

    QLabel *serialLabel = new QLabel(tr("Serial number: %1").arg(serial));
    verticalLayout->addWidget(serialLabel);

    QLabel *fwVerLabel = new QLabel(tr("Firmware version: %1").arg(firmwareVersion));
    verticalLayout->addWidget(fwVerLabel);
}

QDBusObjectPath DeviceWidget::getDevicePath()
{
    return devicePath;
}

DeviceWidget::~DeviceWidget() = default;

void DeviceWidget::pollCombo(int /* index */)
{
    auto *sender = qobject_cast<QComboBox *>(QObject::sender());
    try {
        device->setPollRate(sender->currentData().value<ushort>());
    } catch (const libopenrazer::DBusException &e) {
        qWarning("Failed to set polling rate");
        util::showError(tr("Failed to set polling rate"));
    }
}

void DeviceWidget::openCustomEditor(bool openMatrixDiscovery)
{
    // Set combobox(es) to "Custom Effect"
    auto comboboxes = this->findChildren<QComboBox *>("combobox");
    for (auto combobox : comboboxes) {
        if (combobox->findText("Custom Effect") == -1)
            combobox->addItem("Custom Effect");
        combobox->setCurrentText("Custom Effect");
    }

    auto *cust = new CustomEditor(device, openMatrixDiscovery);
    cust->setAttribute(Qt::WA_DeleteOnClose);
    cust->show();
}
