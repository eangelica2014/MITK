/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPartialVolumeAnalysisWidget.h"

#include "mitkHistogramGenerator.h"
#include "mitkPartialVolumeAnalysisClusteringCalculator.h"

#include <qlabel.h>
#include <qpen.h>
#include <qgroupbox.h>


#include <vtkQtChartArea.h>
#include <vtkQtChartTableSeriesModel.h>
#include <vtkQtChartStyleManager.h>
#include <vtkQtChartColorStyleGenerator.h>

#include <vtkQtChartMouseSelection.h>
#include <vtkQtChartInteractorSetup.h>
#include <vtkQtChartSeriesSelectionHandler.h>
#include <vtkQtChartAxisLayer.h>
#include <vtkQtChartAxis.h>
#include <vtkQtChartAxisOptions.h>
#include <vtkQtChartLegend.h>
#include <vtkQtChartLegendManager.h>

//#include <iostream>

QmitkPartialVolumeAnalysisWidget::QmitkPartialVolumeAnalysisWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
//  this->SetAxisTitle( QwtPlot::xBottom, "Grayvalue" );
  //  this->SetAxisTitle( QwtPlot::yLeft, "Probability" );
//  this->Replot();
  m_Plot->setCanvasLineWidth(0);
  m_Plot->setMargin(0);
}



QmitkPartialVolumeAnalysisWidget::~QmitkPartialVolumeAnalysisWidget()
{
}


void QmitkPartialVolumeAnalysisWidget::DrawGauss()
{

}


void QmitkPartialVolumeAnalysisWidget::ClearItemModel()
{

}

void QmitkPartialVolumeAnalysisWidget::SetParameters( ParamsType *params, ResultsType *results, HistType *hist )
{
  this->Clear();

  if(params != 0 && results != 0)
  {
    params->Print();

    for(unsigned int i=0; i<m_Vals.size(); i++)
    {
      delete m_Vals[i];
    }

    m_Vals.clear();
    m_Vals.push_back(hist->GetXVals());
    m_Vals.push_back(hist->GetHVals());

    QPen pen( Qt::SolidLine );
    pen.setWidth(2);

    pen.setColor(Qt::black);
    int curveId = this->InsertCurve( "histogram" );
    this->SetCurveData( curveId, (*m_Vals[0]), (*m_Vals[1]) );
    this->SetCurvePen( curveId, pen );
    //  this->SetCurveTitle( curveId, "Image Histogram" );

    std::vector<double> *fiberVals = new std::vector<double>(results->GetFiberVals());
    curveId = this->InsertCurve( "fiber" );
    this->SetCurveData( curveId, (*hist->GetXVals()), (*fiberVals) );
    this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
    this->SetCurveBrush(curveId, QBrush(QColor::fromRgbF(0,1,0,.5), Qt::SolidPattern));
    m_Vals.push_back(fiberVals);

    std::vector<double> *nonFiberVals = new std::vector<double>(results->GetNonFiberVals());
    curveId = this->InsertCurve( "nonfiber" );
    this->SetCurveData( curveId, (*hist->GetXVals()), (*nonFiberVals) );
    this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
    this->SetCurveBrush(curveId, QBrush(QColor::fromRgbF(1,0,0,.5), Qt::SolidPattern));
    m_Vals.push_back(nonFiberVals);

    std::vector<double> *mixedVals = new std::vector<double>(results->GetMixedVals());
    curveId = this->InsertCurve( "mixed" );
    this->SetCurveData( curveId, (*hist->GetXVals()), (*mixedVals) );
    this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
    this->SetCurveBrush(curveId, QBrush(QColor::fromRgbF(.7,.7,.7,.5), Qt::SolidPattern));
    m_Vals.push_back(mixedVals);

    pen.setColor(Qt::blue);
    std::vector<double> *combiVals = new std::vector<double>(results->GetCombiVals());
    curveId = this->InsertCurve( "combi" );
    this->SetCurveData( curveId, (*hist->GetXVals()), (*combiVals) );
    this->SetCurvePen( curveId, pen );
    m_Vals.push_back(combiVals);



  }

  this->Replot();

}
