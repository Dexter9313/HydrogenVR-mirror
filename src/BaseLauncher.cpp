/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@epfl.ch>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "BaseLauncher.hpp"

void BaseLauncher::init()
{
	this->setWindowTitle(QString(PROJECT_NAME) + tr(" Launcher"));
	this->setFixedSize(QSize(800, 600));

	mainLayout = new QVBoxLayout(this);

	// SETTINGS TAB WIDGET
	settingsWidget = newSettingsWidget();
	mainLayout->addWidget(settingsWidget);

	// LAUNCH AND QUIT BUTTONS
	auto w = new QWidget(this);
	mainLayout->addWidget(w);
	auto l   = new QHBoxLayout(w);
	auto pbl = new QPushButton(this);
	l->addWidget(pbl);
	pbl->setText(tr("LAUNCH"));
	pbl->setDefault(true);
	connect(pbl, SIGNAL(pressed()), this, SLOT(accept()));

	auto pbr = new QPushButton(this);
	l->addWidget(pbr);
	pbr->setText(tr("RESET TO DEFAULT"));
	connect(pbr, SIGNAL(pressed()), this, SLOT(resetSettings()));

	auto pbq = new QPushButton(this);
	l->addWidget(pbq);
	pbq->setText(tr("QUIT"));
	connect(pbq, SIGNAL(pressed()), this, SLOT(reject()));
}

SettingsWidget* BaseLauncher::newSettingsWidget()
{
	return new SettingsWidget(this);
}

void BaseLauncher::resetSettings()
{
	if(QMessageBox::warning(
	       this, tr("Resetting to default"),
	       tr("Are you sure you want to reset your settings to default ?"),
	       QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel)
	   == QMessageBox::Cancel)
	{
		return;
	}
	QSettings().clear();
	SettingsWidget* oldWidget(settingsWidget);
	settingsWidget = newSettingsWidget();
	mainLayout->replaceWidget(oldWidget, settingsWidget);
	delete oldWidget;
}
