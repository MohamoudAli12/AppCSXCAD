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

#ifndef APPTHEME_H
#define APPTHEME_H

//! Light/dark color theme handling for AppCSXCAD.
namespace AppTheme
{
	enum Mode
	{
		System, Light, Dark
	};

	//! Read/write the stored theme preference (QSettings group "Appearance")
	Mode storedMode();
	void setStoredMode(Mode mode);

	//! Read/write the "keep the 3D view background light" override
	bool storedLightViewport();
	void setStoredLightViewport(bool val);

	//! Resolve a mode to a dark/light decision, asking the desktop for Mode System
	bool isDark(Mode mode);

	//! Apply a mode to the QApplication style and palette
	void apply(Mode mode);

	//! Apply the stored preference. Call this before creating any widget,
	//! otherwise the window is briefly painted with the wrong palette.
	void applyStored();
}

#endif // APPTHEME_H
