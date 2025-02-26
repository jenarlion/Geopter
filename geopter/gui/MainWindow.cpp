#include <iostream>
#include <limits>
#include <sstream>
#include <fstream>

#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QTextEdit>
#include <QShortcut>
#include <QDebug>

#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "GeneralConfigDlg.h"

#include "renderer_qcp.h"

#include "SystemEditor/SystemDataConstant.h"

#include "Dock/TextViewDock.h"

#include "AnalysisDlg/AnalysisSettingDlg.h"
#include "AnalysisDlg/Layout2dDlg.h"
#include "AnalysisDlg/TransverseRayFanDlg.h"
#include "AnalysisDlg/SingleRayTraceDlg.h"
#include "AnalysisDlg/LongitudinalDlg.h"
#include "AnalysisDlg/ParaxialTraceDlg.h"
#include "AnalysisDlg/PrescriptionDlg.h"
#include "AnalysisDlg/FieldCurvatureDlg.h"
#include "AnalysisDlg/ChromaticFocusShiftDlg.h"
#include "AnalysisDlg/SpotDiagramDlg.h"
#include "AnalysisDlg/OpdFanDlg.h"
#include "AnalysisDlg/WavefrontMapDlg.h"
#include "AnalysisDlg/FFT_PSFDlg.h"
#include "AnalysisDlg/GeoMtfDlg.h"
#include "AnalysisDlg/FFT_MTFDlg.h"

using namespace ads;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Geopter");

    // File menu
    QObject::connect(ui->actionNew,  SIGNAL(triggered()), this, SLOT(newFile()));
    QObject::connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveAs()));
    QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    QObject::connect(ui->actionPreference, SIGNAL(triggered()), this, SLOT(showPreference()));

    // Edit menu
    QObject::connect(ui->actionSetVigFactors, SIGNAL(triggered()), this, SLOT(setVignettingFactors()));

    // Analysis menu
    QObject::connect(ui->actionPrescription,           SIGNAL(triggered()), this, SLOT(showPrescription()));
    QObject::connect(ui->action2DLayout,               SIGNAL(triggered()), this, SLOT(showLayout()));
    QObject::connect(ui->actionSingleRayTrace,         SIGNAL(triggered()), this, SLOT(showSingleRayTrace()));
    QObject::connect(ui->actionParaxialRayTrace,       SIGNAL(triggered()), this, SLOT(showParaxialRayTrace()));
    QObject::connect(ui->actionTransverseRayFan,       SIGNAL(triggered()), this, SLOT(showTransverseRayFan()));
    QObject::connect(ui->actionSpherochromatism,       SIGNAL(triggered()), this, SLOT(showLongitudinal()));
    QObject::connect(ui->actionFieldCurvature,         SIGNAL(triggered()), this, SLOT(showFieldCurvature()));
    QObject::connect(ui->actionChromaticFocusShift,    SIGNAL(triggered()), this, SLOT(showChromaticFocusShift()));
    QObject::connect(ui->actionSpotDiagram,            SIGNAL(triggered()), this, SLOT(showSpotDiagram()));
    QObject::connect(ui->actionOpdFan,                 SIGNAL(triggered()), this, SLOT(showOpdFan()));
    QObject::connect(ui->actionWavefrontMap,           SIGNAL(triggered()), this, SLOT(showWavefront()));
    QObject::connect(ui->actionFFT_PSF,                SIGNAL(triggered()), this, SLOT(showFFTPSF()));
    QObject::connect(ui->actionGeoMTF,                 SIGNAL(triggered()), this, SLOT(showGeoMTF()));
    QObject::connect(ui->actionFFT_MTF,                SIGNAL(triggered()), this, SLOT(showFFTMTF()));

    // Tool
    QObject::connect(ui->actionConsole,                SIGNAL(triggered()), this, SLOT(showConsole()));

    // Help menu
    QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

    // keyboard shortcut

    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);

    // create optical system
    opt_sys_ = std::make_shared<QOpticalSystem>();
    opt_sys_->initialize();

    // set system editor as central dock
    m_dockManager = new CDockManager(this);

    m_systemEditorDock = new SystemEditorDock(opt_sys_, "System Editor");
    auto* CentralDockArea = m_dockManager->setCentralWidget(m_systemEditorDock );
    CentralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);


    // load glass catalogs
    QString agfDir = QApplication::applicationDirPath() + "/AGF";
    loadAgfsFromDir(agfDir);

}

MainWindow::~MainWindow()
{
    opt_sys_.reset();


    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Delete dock manager here to delete all floating widgets. This ensures
    // that all top level windows of the dock manager are properly closed
    m_dockManager->deleteLater();
    QMainWindow::closeEvent(event);
}


void MainWindow::loadAgfsFromDir(QString agfDir)
{
    std::vector< std::string > agf_paths;

    QStringList nameFilters;
    nameFilters.append("*.agf");
    nameFilters.append("*.AGF");

    QDir dir(agfDir);
    QStringList entry = dir.entryList(nameFilters, QDir::Files);
    for (QString &file : entry) {
        qDebug() << dir.filePath(file);
        agf_paths.push_back(dir.filePath(file).toStdString());
    }

    bool ret = opt_sys_->material_lib()->load_agf_files(agf_paths);
    if(!ret){
        QMessageBox::warning(this,tr("Error") ,tr("AGF load error"));
    }

}


void MainWindow::syncUiWithSystem()
{
    //m_systemEditorDock->updateUi();
}

/*********************************************************************************************************************************
 *
 * File menu
 *
 * ********************************************************************************************************************************/
void MainWindow::newFile()
{
    opt_sys_->initialize();
    //opt_sys_->update_model();

    m_systemEditorDock->setOpticalSystem(opt_sys_);

    QMessageBox::information(this,tr("Info"), tr("Created new lens"));
}

void MainWindow::saveAs()
{
    // open file selection dialog
    QString filePath = QFileDialog::getSaveFileName(this, tr("select JSON"), QApplication::applicationDirPath(), tr("JSON files(*.json);;All Files(*.*)"));
    if(filePath.isEmpty()){
        return;
    }

    std::string json_path = filePath.toStdString();
    opt_sys_->save_to_file(json_path);

    QMessageBox::information(this,tr("Info"), tr("Saved to JSON file"));
}

void MainWindow::openFile()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open JSON"), QApplication::applicationDirPath(),tr("JSON files(*.json)"));
    if(filePaths.empty()){
        //QMessageBox::warning(this,tr("Canceled"), tr("Canceled"));
        return;
    }

    std::string json_path = filePaths.first().toStdString();
    opt_sys_->load_file(json_path);

    opt_sys_->update_model();

    m_systemEditorDock->setOpticalSystem(opt_sys_);
    m_systemEditorDock->rebuildUi();

    QMessageBox::information(this,tr("Info"), tr("OpticalSystem newly loaded"));
}

void MainWindow::showPreference()
{
    GeneralConfigDlg* dlg = new GeneralConfigDlg(opt_sys_, this);
    dlg->exec();
    delete dlg;
}

/*********************************************************************************************************************************
 *
 * Edit menu
 *
 * ********************************************************************************************************************************/
void MainWindow::setVignettingFactors()
{
    opt_sys_->set_vignetting_factors();
    opt_sys_->update_model();

    // In order to update UI, emit signal by running a simple function
    int stop = opt_sys_->optical_assembly()->stop_index();
    m_systemEditorDock->systemEditorWidget()->lensDataView()->lensDataModel()->setStop(stop);
}


/*********************************************************************************************************************************
 *
 * Analysis menu
 *
 * ********************************************************************************************************************************/
void MainWindow::showPrescription()
{    
    constexpr bool textOnly = true;
    showAnalysisResult<PrescriptionDlg>("Prescription", textOnly);
}

void MainWindow::showLayout()
{    
    showAnalysisResult<Layout2dDlg>("2D Layout");
}

void MainWindow::showParaxialRayTrace()
{
    showAnalysisResult<ParaxialTraceDlg>("Paraxial Ray Trace");
}

void MainWindow::showSingleRayTrace()
{
    showAnalysisResult<SingleRayTraceDlg>("Single Ray Trace");
}

void MainWindow::showSpotDiagram()
{
    showAnalysisResult<SpotDiagramDlg>("Spot Diagram");
}

void MainWindow::showTransverseRayFan()
{
    showAnalysisResult<TransverseRayFanDlg>("Transverse Aberration");
}

void MainWindow::showOpdFan()
{
    showAnalysisResult<OpdFanDlg>("OPD Fan");
}

void MainWindow::showLongitudinal()
{    
    showAnalysisResult<LongitudinalDlg>("Longitudinal Aberration");
}

void MainWindow::showFieldCurvature()
{
    showAnalysisResult<FieldCurvatureDlg>("Field Curvature");
}

void MainWindow::showChromaticFocusShift()
{
    showAnalysisResult<ChromaticFocusShiftDlg>("Chromatic Focus Shift");
}

void MainWindow::showWavefront()
{
    showAnalysisResult<WavefrontMapDlg>("Wavefront Map");
}

void MainWindow::showFFTPSF()
{
    QMessageBox::information(this,tr("Info"), tr("This function is temporally unavailable."));
    //showAnalysisPlot<FFT_PSFDlg>("FFT PSF");
}

void MainWindow::showGeoMTF()
{
    QMessageBox::information(this,tr("Info"), tr("This is a test function."));
    showAnalysisResult<GeoMtfDlg>("Geometrical MTF");
}

void MainWindow::showFFTMTF()
{
    QMessageBox::information(this,tr("Info"), tr("This function is temporally unavailable."));
    //showAnalysisPlot<FFT_MTFDlg>("FFT MTF");
}

template<class T>
void MainWindow::showAnalysisPlot(QString dockTitleBase)
{
    QString dockTitleWithNumber = createDockTitleWithNumber(dockTitleBase);
    PlotViewDock *dock = new PlotViewDock(dockTitleWithNumber, opt_sys_.get());
    dock->createSettingDialog<T>();
    m_dockManager->addDockWidgetFloating(dock);
    ui->menuView->addAction(dock->toggleViewAction());
    //dock->resize(300,200);
    dock->updateContent();
}


template<class T>
void MainWindow::showAnalysisText(QString dockTitleBase)
{
    QString dockTitleWithNumber = createDockTitleWithNumber(dockTitleBase);
    TextViewDock *dock = new TextViewDock(dockTitleWithNumber, opt_sys_.get());
    dock->createSettingDialog<T>();
    m_dockManager->addDockWidgetFloating(dock);
    ui->menuView->addAction(dock->toggleViewAction());
    //dock->resize(300,200);
    dock->updateContent();
}

template <class T>
void MainWindow::showAnalysisResult(QString dockTitleBase, bool textOnly)
{
    QString dockTitleWithNumber = createDockTitleWithNumber(dockTitleBase);
    AnalysisViewDock *dock = new AnalysisViewDock(dockTitleWithNumber, opt_sys_.get(), textOnly);
    dock->createSettingDialog<T>();
    m_dockManager->addDockWidgetFloating(dock);
    ui->menuView->addAction(dock->toggleViewAction());
    //dock->resize(300,200);
    dock->updateContent();
}


QString MainWindow::createDockTitleWithNumber(QString dockTitleBase)
{
    if(m_dockManager->dockWidgetsMap().contains(dockTitleBase))
    {
        QString dockTitleWithNumber;
        int n = 1;
        while(true)
        {
            dockTitleWithNumber = dockTitleBase + "_" + QString::number(n);

            if(m_dockManager->dockWidgetsMap().contains(dockTitleWithNumber)){
                n += 1;
            }else{
                break;
            }
        }

        return dockTitleWithNumber;

    }else{
        return dockTitleBase;
    }

}


/*********************************************************************************************************************************
 *
 * Tool menu
 *
 * ********************************************************************************************************************************/
void MainWindow::showConsole()
{
    QTabWidget *consoleTab = new QTabWidget;
    m_pyConsole = new PythonQtScriptingConsole(NULL, PythonQt::self()->getMainModule());
    QTextEdit* stdoutText = new QTextEdit;
    stdoutText->setReadOnly(true);
    stdoutText->setLineWrapMode(QTextEdit::NoWrap);
    QTextEdit* stderrText = new QTextEdit;
    stderrText->setReadOnly(true);
    stderrText->setLineWrapMode(QTextEdit::NoWrap);
    consoleTab->addTab(m_pyConsole, tr("PyConsole"));
    consoleTab->addTab(stdoutText, tr("Output"));
    consoleTab->addTab(stderrText, tr("Error"));
    m_qout = new QDebugStream(std::cout, stdoutText);
    m_qerr = new QDebugStream(std::cerr, stderrText);

    CDockWidget* ConsoleDock = new CDockWidget("Console");

    ConsoleDock->setWidget(consoleTab);
    //ConsoleDock->setFeature(CDockWidget::DockWidgetClosable, false);
    m_dockManager->addDockWidgetFloating(ConsoleDock);
    ui->menuView->addAction(ConsoleDock->toggleViewAction());

    QString scriptDirPath = QApplication::applicationDirPath() + "/scripts";
    PythonQt::self()->getMainModule().evalScript("import sys");
    PythonQt::self()->getMainModule().evalScript("sys.path.append(\"" + scriptDirPath +"\" )");

    PythonQt::self()->getMainModule().addObject("osys",opt_sys_.get());
    PythonQt::self()->getMainModule().addObject("lde",opt_sys_->LensDataEditor());
}


/*********************************************************************************************************************************
 *
 * Help menu
 *
 * ********************************************************************************************************************************/
void MainWindow::showAbout()
{
    QString text =
                "Geopter  -Optical Design Software-<br><br>"
                "This program is distributed in the hope that it will be useful,\n"
                "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                "GNU General Public License for more details.<br><br>"
                "Copyright(C) 2021 - present  Hiiragi<br><br>"
                "Contact : heterophyllus.work@gmail.com<br><br>"
                "Please see: "
                "<a href=\"https://github.com/heterophyllus/Geopter\">"
                "https://github.com/heterophyllus/Geopter</a></font>";

    QMessageBox msgBox(this);
    msgBox.setText(text);
    msgBox.setWindowTitle(tr("About Geopter"));
    msgBox.exec();

}

void MainWindow::updateUi()
{
    m_systemEditorDock->rebuildUi();
}
