/*
*	Copyright (C) 2012-2015 Thorsten Liebig (Thorsten.Liebig@gmx.de)
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef APPCSXCAD_H
#define APPCSXCAD_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include "QCSXCAD.h"

#define __APPNAME__ "AppCSXCAD"

#include "AppTheme.h"

class QAction;
class QActionGroup;
class QMenuBar;

class AppCSXCAD : public QCSXCAD
{
    Q_OBJECT

public:
    AppCSXCAD(QWidget *parent = 0);
    virtual ~AppCSXCAD();

    virtual bool ReadFile(QString filename);

public slots:
	void Save();
	virtual void clear();

protected slots:
	//! Apply the theme carried by the sending View menu action
	void SetThemeMode();
	//! Keep the 3D view background light regardless of the theme
	void SetLightViewport(bool val);

protected:
	void parseCommandLineArguments(const QStringList &argList);
	void SaveSettings();
	void LoadSettings();
	QString m_filename;

	void BuildViewMenu(QMenuBar* mb);
	//! Push the current theme/override to the 3D view background
	void ApplyViewportTheme();

	AppTheme::Mode m_ThemeMode;
	bool m_LightViewport;
	QActionGroup* m_ThemeGroup;
	QAction* m_LightViewportAction;
};

#endif // APPCSXCAD_H
