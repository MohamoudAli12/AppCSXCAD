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

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

#include "AppCSXCAD.h"
#include "AppTheme.h"

#include "QVTKStructure.h"
#include "QCSXCAD_Global.h"

//! add one checkable theme choice to the View menu
static void AddThemeAction(QMenu* menu, QActionGroup* group, const QString &text,
                           AppTheme::Mode mode, AppTheme::Mode current, QObject* receiver)
{
	QAction* act = menu->addAction(text);
	act->setCheckable(true);
	act->setData((int)mode);
	act->setChecked(mode==current);
	group->addAction(act);
	QObject::connect(act,SIGNAL(triggered()),receiver,SLOT(SetThemeMode()));
}

AppCSXCAD::AppCSXCAD(QWidget *parent) : QCSXCAD(parent)
{
	//the palette itself was already applied by AppTheme::applyStored() in main(),
	//here we only need the values to drive the menu and the 3D view
	m_ThemeMode = AppTheme::storedMode();
	m_LightViewport = AppTheme::storedLightViewport();
	m_ThemeGroup = NULL;
	m_LightViewportAction = NULL;

	QMenuBar* mb = menuBar();
	QMenu *FileMenu = mb->addMenu("&File");

	if (QCSX_Settings.GetEdit())
		FileMenu->addAction(QIcon(":/images/filenew.png"),tr("New"),this,SLOT(New()),QKeySequence(tr("Ctrl+N")));
	FileMenu->addAction(QIcon(":/images/fileopen.png"),tr("Load"),this,SLOT(ImportGeometry()),QKeySequence(tr("Ctrl+L")));
	FileMenu->addSeparator();
	if (QCSX_Settings.GetEdit())
	{
		FileMenu->addAction(QIcon(":/images/filesave.png"),tr("Save"),this,SLOT(Save()),QKeySequence(tr("Ctrl+S")));
		FileMenu->addAction(QIcon(":/images/filesave.png"),tr("Save As"),this,SLOT(ExportGeometry()));
	}
	//FileMenu->addAction(QIcon(":/images/filesaveas.png"),tr("Save As.."),this,SLOT(SaveAs()));
	QMenu *FileExportMenu = FileMenu->addMenu(tr("E&xport"));
	FileExportMenu->addAction(QIcon(":/images/filesave.png"),tr("CSXCAD XML"),this,SLOT(ExportGeometry()));
	FileExportMenu->addAction(QIcon(":/images/filesave.png"),tr("Pov&ray"),this,SLOT(ExportGeometry_Povray()),QKeySequence(tr("Ctrl+R")));
	FileExportMenu->addAction(QIcon(":/images/filesave.png"),tr("Polydata-&VTK"),this,SLOT(ExportGeometry_PolyDataVTK()));
	FileExportMenu->addAction(QIcon(":/images/filesave.png"),tr("STL"),this,SLOT(ExportGeometry_STL()));
	FileExportMenu->addAction(QIcon(":/images/filesave.png"),tr("PLY"),this,SLOT(ExportGeometry_PLY()));
	FileExportMenu->addAction(QIcon(":/images/filesave.png"),tr("3D view to &PNG"),this,SLOT(ExportView2Image()),QKeySequence(tr("Ctrl+P")));
	FileExportMenu->addAction(QIcon(":/images/filesave.png"),tr("&X3D"),this,SLOT(ExportGeometry_X3D()),QKeySequence(tr("Ctrl+3")));
	FileMenu->addSeparator();
	FileMenu->addAction(QIcon(":/images/exit.png"),tr("Exit"),qApp,SLOT(quit()));

	BuildViewMenu(mb);

	QMenu *InfoMenu = mb->addMenu("Info");
	InfoMenu->addAction(tr("About"),this,SLOT(aboutQCSXCAD()));

	QStringList argList=qApp->arguments();
	if (argList.size()>1)
	{
		QString file = argList.back();
		if (!file.startsWith("-"))
			if (ReadFile(file)==false)
				QMessageBox::information(this,"File Error!",tr("Can't open file: %1").arg(file));
		GUIUpdate();
	}

	QString title = tr("AppCSXCAD");
	if (QCSX_Settings.GetEdit()==false)
		title += " - " + tr("View Mode");
	setWindowTitle(title);
	View3D();
	ApplyViewportTheme();
	LoadSettings();

	parseCommandLineArguments(qApp->arguments());
}

AppCSXCAD::~AppCSXCAD()
{
	SaveSettings();
}

void AppCSXCAD::BuildViewMenu(QMenuBar* mb)
{
	QMenu* ViewMenu = mb->addMenu(tr("&View"));

	m_ThemeGroup = new QActionGroup(this);
	m_ThemeGroup->setExclusive(true);
	AddThemeAction(ViewMenu,m_ThemeGroup,tr("System Theme"),AppTheme::System,m_ThemeMode,this);
	AddThemeAction(ViewMenu,m_ThemeGroup,tr("Light Theme"),AppTheme::Light,m_ThemeMode,this);
	AddThemeAction(ViewMenu,m_ThemeGroup,tr("Dark Theme"),AppTheme::Dark,m_ThemeMode,this);

	ViewMenu->addSeparator();

	//the 3D background is baked into "Export 3D view to PNG", so it needs its own switch
	m_LightViewportAction = ViewMenu->addAction(tr("Light 3D Background"));
	m_LightViewportAction->setCheckable(true);
	m_LightViewportAction->setChecked(m_LightViewport);
	connect(m_LightViewportAction,SIGNAL(toggled(bool)),this,SLOT(SetLightViewport(bool)));

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	//follow the desktop live while "System Theme" is selected
	connect(qApp->styleHints(),&QStyleHints::colorSchemeChanged,this,[this](Qt::ColorScheme)
	{
		if (m_ThemeMode!=AppTheme::System) return;
		AppTheme::apply(AppTheme::System);
		ApplyViewportTheme();
	});
#endif
}

void AppCSXCAD::SetThemeMode()
{
	QAction* act = qobject_cast<QAction*>(sender());
	if (act==NULL) return;

	m_ThemeMode = (AppTheme::Mode)act->data().toInt();
	AppTheme::setStoredMode(m_ThemeMode);
	AppTheme::apply(m_ThemeMode);
	ApplyViewportTheme();
}

void AppCSXCAD::SetLightViewport(bool val)
{
	m_LightViewport = val;
	AppTheme::setStoredLightViewport(val);
	ApplyViewportTheme();
}

void AppCSXCAD::ApplyViewportTheme()
{
	if (StructureVTK==NULL) return;

	if (AppTheme::isDark(m_ThemeMode) && !m_LightViewport)
		StructureVTK->SetBackgroundColor(45,45,45);
	else
		StructureVTK->SetBackgroundColor(255,255,255);
}

void AppCSXCAD::parseCommandLineArguments(const QStringList &argList)
{
	for (int n=0;n<argList.size();++n)
	{
		QString arg = argList.at(n);
		if (!arg.startsWith("--"))
			continue;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
		QStringList arg_split = arg.split("=",Qt::SkipEmptyParts);
#else
		QStringList arg_split = arg.split("=",QString::SkipEmptyParts);
#endif

		if (arg_split.size()==0)
			continue;

		//compate with all known args
		if (arg_split.at(0).compare("--export-polydata-vtk")==0)
		{
			std::cout << "AppCSXCAD - exporting 3D view to polydata vtk" << std::endl;
			if (arg_split.size()<=1)
				continue;
			ExportGeometry_PolyDataVTK(arg_split.at(1));
		}
		if (arg_split.at(0).compare("--export-STL")==0)
		{
			std::cout << "AppCSXCAD - exporting 3D data to STL" << std::endl;
			if (arg_split.size()<=1)
				continue;
			ExportGeometry_STL(arg_split.at(1));
		}
	}
}

void AppCSXCAD::SaveSettings()
{
	QSettings settings(__APPNAME__,__APPNAME__);
	settings.beginGroup("MainWindow");
	settings.setValue("Geometry",saveGeometry());
	settings.setValue("State",saveState());
	settings.endGroup();
}

void AppCSXCAD::LoadSettings()
{
	QSettings settings(__APPNAME__, __APPNAME__);
	settings.beginGroup("MainWindow");
	restoreGeometry(settings.value("Geometry").toByteArray());
	//restoreState(settings.value("State").toByteArray());
	settings.endGroup();
}


bool AppCSXCAD::ReadFile(QString filename)
{
	m_filename=filename;
	return QCSXCAD::ReadFile(filename);
}

void AppCSXCAD::Save()
{
	Write2XML(m_filename);
}

void AppCSXCAD::clear()
{
	m_filename=QString();
	QCSXCAD::clear();
}

