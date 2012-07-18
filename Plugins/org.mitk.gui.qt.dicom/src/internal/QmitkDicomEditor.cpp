/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPartListener.h>
#include <mitkGlobalInteraction.h>
#include <mitkDataStorageEditorInput.h>
#include "berryFileEditorInput.h"

// Qmitk
#include "QmitkDicomEditor.h"
#include "mitkPluginActivator.h"
#include <mitkDicomSeriesReader.h>

//#include "mitkProgressBar.h"

// Qt
#include <QCheckBox>
#include <QMessageBox>
#include <QWidget>

#include <QtSql>
#include <QSqlDatabase>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

//CTK
#include <ctkDICOMModel.h>
#include <ctkDICOMAppWidget.h>
#include <ctkDICOMQueryWidget.h>
#include <ctkFileDialog.h>
#include <ctkDICOMQueryRetrieveWidget.h>


const std::string QmitkDicomEditor::EDITOR_ID = "org.mitk.editors.dicomeditor";


QmitkDicomEditor::QmitkDicomEditor()
: m_Thread(new QThread())
, m_DicomDirectoryListener(new QmitkDicomDirectoryListener())
, m_StoreSCPLauncher(new QmitkStoreSCPLauncher(&builder))
, m_Publisher(new QmitkDicomDataEventPublisher())
{
}

QmitkDicomEditor::~QmitkDicomEditor()
{
    m_Thread->quit();
    m_Thread->wait(1000);
    delete m_Handler;
    delete m_Publisher;
    delete m_StoreSCPLauncher;
    delete m_Thread;
    delete m_DicomDirectoryListener;
}

void QmitkDicomEditor::CreateQtPartControl(QWidget *parent )
{   
    m_Controls.setupUi( parent );
    TestHandler();

    SetPluginDirectory();
    SetDatabaseDirectory("DatabaseDirectory");
    SetListenerDirectory("ListenerDirectory");
    StartDicomDirectoryListener();

    m_Controls.m_ctkDICOMQueryRetrieveWidget->useProgressDialog(false);
    m_Controls.StoreSCPLabel->setVisible(false);

    connect(m_Controls.externalDataWidget,SIGNAL(SignalAddDicomData(const QString&)),m_Controls.internalDataWidget,SLOT(StartDicomImport(const QString&)));
    connect(m_Controls.externalDataWidget,SIGNAL(SignalAddDicomData(const QStringList&)),m_Controls.internalDataWidget,SLOT(StartDicomImport(const QStringList&)));
    connect(m_Controls.externalDataWidget,SIGNAL(SignalDicomToDataManager(const QStringList&)),this,SLOT(OnViewButtonAddToDataManager(const QStringList&)));
    connect(m_Controls.externalDataWidget,SIGNAL(SignalChangePage(int)), this, SLOT(OnChangePage(int)));

    connect(m_Controls.internalDataWidget,SIGNAL(FinishedImport(const QString&)),this,SLOT(OnDicomImportFinished(const QString&)));
    connect(m_Controls.internalDataWidget,SIGNAL(FinishedImport(const QStringList&)),this,SLOT(OnDicomImportFinished(const QStringList&)));
    connect(m_Controls.internalDataWidget,SIGNAL(SignalDicomToDataManager(const QStringList&)),this,SLOT(OnViewButtonAddToDataManager(const QStringList&)));

    connect(m_Controls.CDButton, SIGNAL(clicked()), m_Controls.externalDataWidget, SLOT(OnFolderCDImport()));
    connect(m_Controls.FolderButton, SIGNAL(clicked()), m_Controls.externalDataWidget, SLOT(OnFolderCDImport()));
    connect(m_Controls.FolderButton, SIGNAL(clicked()), this, SLOT(OnFolderCDImport()));
    connect(m_Controls.QueryRetrieveButton, SIGNAL(clicked()), this, SLOT(OnQueryRetrieve()));
    connect(m_Controls.LocalStorageButton, SIGNAL(clicked()), this, SLOT(OnLocalStorage()));

    //connect(m_Controls.radioButton,SIGNAL(clicked()),this,SLOT(StartStopStoreSCP()));
}

void QmitkDicomEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
    this->SetSite(site);
    this->SetInput(input);
}

void QmitkDicomEditor::SetFocus()
{
}

berry::IPartListener::Events::Types QmitkDicomEditor::GetPartEventTypes() const
{
    return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkDicomEditor::OnQueryRetrieve()
{
    OnChangePage(2);
    QString storagePort = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
    QString storageAET = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
     if(!((builder.GetAETitle()->compare(storageAET,Qt::CaseSensitive)==0)&&
         (builder.GetPort()->compare(storagePort,Qt::CaseSensitive)==0)))
     {
         StopStoreSCP();
         StartStoreSCP();
     }
    m_Controls.StoreSCPLabel->setVisible(true);
}

void QmitkDicomEditor::OnFolderCDImport()
{
    m_Controls.StoreSCPLabel->setVisible(false);
}

void QmitkDicomEditor::OnLocalStorage()
{
    OnChangePage(0);
    m_Controls.StoreSCPLabel->setVisible(false);
}

void QmitkDicomEditor::OnChangePage(int page)
{
    try{
        m_Controls.stackedWidget->setCurrentIndex(page);
    }catch(std::exception e){
        MITK_ERROR <<"error: "<< e.what();
        return;
    }
}

void QmitkDicomEditor::OnDicomImportFinished(const QString& /*path*/)
{
}

void QmitkDicomEditor::OnDicomImportFinished(const QStringList& /*path*/)
{
}

void QmitkDicomEditor::StartDicomDirectoryListener()
{   
    if(!m_Thread->isRunning())
    {
        m_DicomDirectoryListener->SetDicomListenerDirectory(m_ListenerDirectory);
        connect(m_DicomDirectoryListener,SIGNAL(SignalAddDicomData(const QStringList&)),m_Controls.internalDataWidget,SLOT(StartDicomImport(const QStringList&)),Qt::DirectConnection);
        connect(m_Controls.internalDataWidget,SIGNAL(FinishedImport(const QStringList&)),m_DicomDirectoryListener,SLOT(OnDicomImportFinished(const QStringList&)),Qt::DirectConnection);
        m_DicomDirectoryListener->moveToThread(m_Thread);
        m_Thread->start();
    }
}

//TODO Remove
void QmitkDicomEditor::TestHandler()
{
    m_Handler = new DicomEventHandler();
    m_Handler->SubscribeSlots();
}

void QmitkDicomEditor::OnViewButtonAddToDataManager(const QStringList& eventProperties)
{
    ctkDictionary properties;
    properties["PatientName"] = eventProperties.at(0);
    properties["StudyUID"] = eventProperties.at(1);
    properties["StudyName"] = eventProperties.at(2);
    properties["SeriesUID"] = eventProperties.at(3);
    properties["SeriesName"] = eventProperties.at(4);
    properties["Path"] = eventProperties.at(5);

    m_Publisher->PublishSignals(mitk::PluginActivator::getContext());
    m_Publisher->AddSeriesToDataManagerEvent(properties);
}


void QmitkDicomEditor::StartStoreSCP()
{
    QString storagePort = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
    QString storageAET = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
    builder.AddPort(storagePort)->AddAETitle(storageAET)->AddTransferSyntax()->AddOtherNetworkOptions()->AddMode()->AddOutputDirectory(m_ListenerDirectory);
    m_StoreSCPLauncher = new QmitkStoreSCPLauncher(&builder);
    m_StoreSCPLauncher->StartStoreSCP();
    m_Controls.StoreSCPLabel->setText("Storage provider is running on port: "+storagePort);

}


void QmitkDicomEditor::StopStoreSCP()
{
        delete m_StoreSCPLauncher;
        m_Controls.StoreSCPLabel->setText(QString("Storage service provider is not running!"));
}

void QmitkDicomEditor::SetPluginDirectory()
{
  m_PluginDirectory = mitk::PluginActivator::getContext()->getDataFile("").absolutePath();
  m_PluginDirectory.append("/");
}

void QmitkDicomEditor::SetDatabaseDirectory(const QString& databaseDirectory)
{
    m_DatabaseDirectory.clear();
    m_DatabaseDirectory.append(m_PluginDirectory);
    m_DatabaseDirectory.append(databaseDirectory);
    m_Controls.internalDataWidget->SetDatabaseDirectory(m_DatabaseDirectory);
}

void QmitkDicomEditor::SetListenerDirectory(const QString& listenerDirectory)
{
    m_ListenerDirectory.clear();
    m_ListenerDirectory.append(m_PluginDirectory);
    m_ListenerDirectory.append(listenerDirectory);
}