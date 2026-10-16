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

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QSettings>
#include <QString>
#include <QStyle>
#include <QStyleFactory>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

#include "AppTheme.h"
#include "AppCSXCAD.h"

namespace
{
	//the style and palette the desktop handed us, captured before we touch anything
	bool     g_NativeCaptured = false;
	QString  g_NativeStyle;
	QPalette g_NativePalette;

	//set once we have overridden the desktop look, so that Mode System can stay
	//completely hands-off until there is actually something to undo
	bool     g_ThemeApplied = false;

	void CaptureNativeLook()
	{
		if (g_NativeCaptured) return;
		g_NativeStyle   = qApp->style()->objectName();
		g_NativePalette = qApp->palette();
		g_NativeCaptured = true;
	}

	QPalette DarkPalette()
	{
		const QColor window(53,53,53);
		const QColor base(35,35,35);
		const QColor text(220,220,220);
		const QColor disabled(127,127,127);
		const QColor highlight(42,130,218);

		QPalette pal;
		pal.setColor(QPalette::Window,window);
		pal.setColor(QPalette::WindowText,text);
		pal.setColor(QPalette::Base,base);
		pal.setColor(QPalette::AlternateBase,window);
		pal.setColor(QPalette::ToolTipBase,window);
		pal.setColor(QPalette::ToolTipText,text);
		pal.setColor(QPalette::Text,text);
		pal.setColor(QPalette::Button,window);
		pal.setColor(QPalette::ButtonText,text);
		pal.setColor(QPalette::BrightText,QColor(255,80,80));
		pal.setColor(QPalette::Link,highlight);
		pal.setColor(QPalette::Highlight,highlight);
		pal.setColor(QPalette::HighlightedText,QColor(0,0,0));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
		pal.setColor(QPalette::PlaceholderText,disabled);
#endif

		//without these, disabled actions (e.g. in --disableEdit view mode) are unreadable
		pal.setColor(QPalette::Disabled,QPalette::WindowText,disabled);
		pal.setColor(QPalette::Disabled,QPalette::Text,disabled);
		pal.setColor(QPalette::Disabled,QPalette::ButtonText,disabled);
		pal.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
		pal.setColor(QPalette::Disabled,QPalette::HighlightedText,disabled);
		return pal;
	}
}

AppTheme::Mode AppTheme::storedMode()
{
	QSettings settings(__APPNAME__,__APPNAME__);
	settings.beginGroup("Appearance");
	QString mode = settings.value("Mode","system").toString();
	settings.endGroup();

	if (mode.compare("dark",Qt::CaseInsensitive)==0) return Dark;
	if (mode.compare("light",Qt::CaseInsensitive)==0) return Light;
	return System;
}

void AppTheme::setStoredMode(Mode mode)
{
	QSettings settings(__APPNAME__,__APPNAME__);
	settings.beginGroup("Appearance");
	if (mode==Dark) settings.setValue("Mode","dark");
	else if (mode==Light) settings.setValue("Mode","light");
	else settings.setValue("Mode","system");
	settings.endGroup();
}

bool AppTheme::storedLightViewport()
{
	QSettings settings(__APPNAME__,__APPNAME__);
	settings.beginGroup("Appearance");
	bool val = settings.value("LightViewport",false).toBool();
	settings.endGroup();
	return val;
}

void AppTheme::setStoredLightViewport(bool val)
{
	QSettings settings(__APPNAME__,__APPNAME__);
	settings.beginGroup("Appearance");
	settings.setValue("LightViewport",val);
	settings.endGroup();
}

bool AppTheme::isDark(Mode mode)
{
	if (mode==Dark) return true;
	if (mode==Light) return false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	return qApp->styleHints()->colorScheme()==Qt::ColorScheme::Dark;
#else
	//no color scheme hint available, judge by the palette the desktop gave us.
	//this has to be the captured one, the live palette is ours once a theme is applied.
	CaptureNativeLook();
	return g_NativePalette.color(QPalette::Window).lightness()<128;
#endif
}

void AppTheme::apply(Mode mode)
{
	CaptureNativeLook();

	//nothing has been overridden yet, so the desktop look is already in place.
	//leaving it untouched also keeps Qt following system palette changes on its own.
	if (mode==System && !g_ThemeApplied) return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
	//tell Qt first: this lets native dialogs (file/color choosers) agree with us
	//instead of fighting, and resetting to Unknown makes the desktop scheme
	//visible again to isDark() for Mode System.
	if (mode==Dark) qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
	else if (mode==Light) qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);
	else qApp->styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
#endif

	if (mode==System)
	{
		//whatever the desktop gives us is the system look, dark desktops included
		QStyle* style = QStyleFactory::create(g_NativeStyle);
		if (style!=NULL) qApp->setStyle(style);
		qApp->setPalette(g_NativePalette);
		return;
	}

	//Light and Dark are explicit choices, so build them from a known style instead
	//of reusing the desktop palette, which on a dark desktop is itself dark
	QStyle* style = QStyleFactory::create("Fusion");
	if (style!=NULL) qApp->setStyle(style);

	if (mode==Dark) qApp->setPalette(DarkPalette());
	else qApp->setPalette(qApp->style()->standardPalette());

	g_ThemeApplied = true;
}

void AppTheme::applyStored()
{
	apply(storedMode());
}
