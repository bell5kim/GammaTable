#include "gammatable.h"
#include "ui_gammatable.h"
#include "user.h"

#include <iostream>
#include <cmath>
using namespace std;

#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QPrinter>
#include <QPainter>
#include <QPaintDevice>
#include <QTextDocument>

GammaTable::GammaTable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GammaTable)
{
    ui->setupUi(this);
}

GammaTable::~GammaTable()
{
    delete ui;
}

// ------------------------------------------------------

InputData *inputData = new InputData;  // This should be declared at very beginning

void GammaTable::init() {
    ui->spinBoxDD->setValue(2);
    ui->spinBoxDTA->setValue(2);
    ui->tableDD->setColumnWidth(0,70);
    ui->tableDD->setColumnWidth(1,70);
    ui->tableDD->setColumnWidth(2,70);
    ui->tableDD->setColumnWidth(3,65);
    ui->tableDD->setColumnWidth(4,65);
    ui->tableDD->setColumnWidth(5,65);
}

void GammaTable::initLoad(char *fname) {
 QString FNAME;
 QTextStream(&FNAME) << fname;
 //QStringList FNAMEList = QStringList::split(':',FNAME, false);
 QStringList FNAMEList = FNAME.split(':');
 //ui->comboBoxFiles->insertStringList(FNAMEList,-1);
 ui->comboBoxFiles->insertItems(-1, FNAMEList);

 //spinBoxDD->setValue(2);
 //spinBoxDTA->setValue(2);
 //tableDD->setColumnWidth(0,70);
 //tableDD->setColumnWidth(1,70);
 //tableDD->setColumnWidth(2,70);
 //tableDD->setColumnWidth(3,65);
 //tableDD->setColumnWidth(4,65);
 //tableDD->setColumnWidth(5,65);
 loadData(FNAMEList[0]);
}

void GammaTable::loadData(QString FNAME) {

 inputData->getInputFile(FNAME);

 ui->tableDD->setRowCount(inputData->nData);

 double dMax = 0; double dRef = 0; int dIndex = 0;
 double xRef = 0.0;

 if (FNAME.contains(".zComp")
  || FNAME.contains("diff.dd")) xRef = 10.0;
 if (FNAME.contains("diff/z")) xRef = 100.0;
 // cout << "nData = " << inputData->nData << endl;
 for (int i=0; i < inputData->nData; i++) {
  QString xText, dcText, dmText;
  QTextStream(&xText)  << inputData->x[i];
  QTextStream(&dcText) << inputData->dc[i];
  QTextStream(&dmText) << inputData->dm[i];
  // cout << inputData->x[i] << " " << inputData->dc[i] << " " << inputData->dm[i] << endl;
  ui->tableDD->setItem(i,0,new QTableWidgetItem(xText));
  ui->tableDD->setItem(i,1,new QTableWidgetItem(dcText));
  ui->tableDD->setItem(i,2,new QTableWidgetItem(dmText));
  if (FNAME.contains(".xComp") || FNAME.contains("diff/x"))
      // ui->tableDD->horizontalHeader()->setLabel(0,"x (cm)");
      ui->tableDD->setHorizontalHeaderItem(0, new QTableWidgetItem("x (cm)"));
  if (FNAME.contains(".yComp") || FNAME.contains("diff/y"))
      // ui->tableDD->horizontalHeader()->setLabel(0,"y (cm)");
      ui->tableDD->setHorizontalHeaderItem(0, new QTableWidgetItem("y (cm)"));
  if (inputData->dm[i]>=dMax) {
   dMax = inputData->dm[i];
   dIndex = i;
  }
  if (i < inputData->nData-1 &&
      (inputData->x[i+1]-xRef)*(inputData->x[i]-xRef) <= 0.0){
    dRef = (fabs(inputData->x[i+1]-xRef)*inputData->dm[i] +
            fabs(inputData->x[i]-xRef)*inputData->dm[i+1])
           / (inputData->x[i+1]-inputData->x[i]);
    // cout << inputData->x[i] << "  " << inputData->dm[i] << "  " << inputData->dm[i+1]  << "  " << dRef << endl;
  }
 }
 // cout << "nData = " << inputData->nData << endl;

 inputData->dRef = dRef;
 inputData->dMax = dMax;
 // cout << "xRef = " << xRef << "  dRef = " << dRef << "  dMax = " << dMax << endl;
 QString refDoseText;
 if (ui->comboBoxRefDose->currentText() == "Dref") dMax = dRef;
 QTextStream(&refDoseText) << dMax;
 ui->lineEditRefDose->setText(refDoseText);

 updateDTA();
 updateDoseDiff();
 // updateGamma();

 ui->dataLabel->setText(makeInfo(FNAME));
}

void GammaTable::updateDoseDiff() {
 QString ddText;
 bool ok;
 double pass=0;
 double dMax =  ui->lineEditRefDose->text().toFloat(&ok);
 int DD = ui->spinBoxDD->value();

 QString refDoseText;

 if (ui->comboBoxRefDose->currentText() == "Dref") dMax = inputData->dRef;
 if (ui->comboBoxRefDose->currentText() == "Dmax") dMax = inputData->dMax;
 QTextStream(&refDoseText) << dMax;
 if (ui->comboBoxRefDose->currentText() == "Pointwise") refDoseText = "Pointwise";
 ui->lineEditRefDose->setText(refDoseText);

 int cutOff = ui->spinBoxCutoff->value();
 float cutOffDose = dMax*cutOff/100.0;
 if (refDoseText.contains("Pointwise")) cutOffDose = inputData->dMax * cutOff/100.0;
 int nCut = 0;

 for (int i=0; i < inputData->nData; i++) {
   if (ui->comboBoxRefDose->currentText() == "Dmax") dMax = inputData->dm[i];
   if (dMax != 0.0) {
     inputData->dd[i] = (inputData->dc[i]-inputData->dm[i]) *100.0/dMax;
     ddText.sprintf("%7.3f", inputData->dd[i]);
     // tableDD->setText(i,3,ddText);
     if (fabs(inputData->dd[i]) > DD) {
       // MyTableItem *tItem = new MyTableItem(ui->tableDD,QTableWidgetItem::Never,ddText);
       ui->tableDD->setItem( i, 3, new QTableWidgetItem(ddText));
       ui->tableDD->item(i, 3)->setBackground(Qt::yellow);
     } else {
       pass++;
       // QTableWidgetItem *tItem = new QTableWidgetItem(ui->tableDD,QTableWidgetItem::Never,ddText);
       ui->tableDD->setItem( i, 3, new QTableWidgetItem(ddText));
         if (inputData->dm[i] <= cutOffDose) pass--;
     }
  }else{
     ui->tableDD->setItem(i,3,new QTableWidgetItem("N/A"));
  }
  if (inputData->dm[i] <= cutOffDose) nCut++;
 }
 QString passText;
 passText.sprintf("%7.3f", (pass/(inputData->nData-nCut)*100) );
 ui->lineEditDdPass->setText(passText);
 updateGamma();
}

void GammaTable::calcDTA() {
 for (int ic=0; ic < inputData->nData; ic++) {
  // dnDTA: DTA in down direction
  double dnDTA = -1000; // Negative value means no DTA
  for (int im=ic; im>0; im--) {
   if( (inputData->dm[im]-inputData->dc[ic])
    *(inputData->dm[im-1]-inputData->dc[ic])<=0.0) {
    dnDTA = inputData->x[ic]
      -(fabs(inputData->dm[im]-inputData->dc[ic])*inputData->x[im-1]
       +fabs(inputData->dm[im-1]-inputData->dc[ic])*inputData->x[im])
      /fabs(inputData->dm[im]-inputData->dm[im-1]);
    dnDTA = fabs(dnDTA);
    // break;
  im = 0;
   }
  }
  double upDTA = -1000; // Negative value means no DTA
  for (int im=ic; im<inputData->nData-1; im++) {
   if( (inputData->dm[im]-inputData->dc[ic])
    *(inputData->dm[im+1]-inputData->dc[ic])<=0.0) {
    upDTA = (fabs(inputData->dm[im]-inputData->dc[ic])*inputData->x[im+1]
        +fabs(inputData->dm[im+1]-inputData->dc[ic])*inputData->x[im])
       /fabs(inputData->dm[im+1]-inputData->dm[im])
        -inputData->x[ic];
    upDTA = fabs(upDTA);
    // break;
    im = inputData->nData;
   }
  }
  inputData->dta[ic] = dnDTA;
  if (fabs(upDTA) < fabs(dnDTA)) inputData->dta[ic] = upDTA;
  inputData->dta[ic] *= 10;  // Convert cm to mm
 }
}

void GammaTable::updateDTA() {
 QString dtaText;
 calcDTA();

 double dMax = 0.0;
 if (ui->comboBoxRefDose->currentText() == "Dref") dMax = inputData->dRef;
 if (ui->comboBoxRefDose->currentText() == "Dmax") dMax = inputData->dMax;
 if (ui->comboBoxRefDose->currentText() == "Pointwise") dMax = inputData->dMax;

 int cutOff = ui->spinBoxCutoff->value();
 float cutOffDose = dMax*cutOff/100.0;
 int nCut = 0;

 double pass=0;
 int DTA = ui->spinBoxDTA->value();
 for (int i=0; i < inputData->nData; i++) {
  if (inputData->dta[i] >= 0) {
   dtaText.sprintf("%7.3f", inputData->dta[i]);
   if (fabs(inputData->dta[i]) > DTA) {  // Failed by DTA
    // MyTableItem *tItem = new MyTableItem(tableDD,QTableWidgetItem::Never,dtaText);
    //   ui->tableDD->setItem( i, 4, tItem );
    ui->tableDD->setItem( i, 4, new QTableWidgetItem(dtaText));
    ui->tableDD->item(i, 4)->setBackground(Qt::yellow);

   } else {
    pass++;
    //QTableWidgetItem *tItem = new QTableWidgetItem(tableDD,QTableWidgetItem::Never,dtaText);
    //ui->tableDD->setItem( i, 4, tItem );
    ui->tableDD->setItem( i, 4, new QTableWidgetItem(dtaText));
     if (inputData->dm[i] <= cutOffDose) pass--;
   }
  }
  if (inputData->dm[i] <= cutOffDose) nCut++;
 }
 QString passText;
 passText.sprintf("%7.3f", (pass/(inputData->nData-nCut)*100) );
 ui->lineEditDtaPass->setText(passText);
}

void GammaTable::calcGamma() {
 int DTA = ui->spinBoxDTA->value();
 int DD  = ui->spinBoxDD->value();
 for (int i=0; i < inputData->nData; i++) {
  if (fabs(inputData->dta[i]) != 10000) {  // DTA exists
   inputData->gamma[i]=sqrt(pow(inputData->dd[i]/DD,2)
            +pow(inputData->dta[i]/DTA,2));
  } else {
   inputData->gamma[i]=-1;  // No Gamma Available
  }
 }
}

void GammaTable::updateGamma() {
 QString gammaText;
 QString passedText("passed");
 calcGamma();
 double pass=0;
 double gPass = 0;
 int DTA = ui->spinBoxDTA->value();
 int DD  = ui->spinBoxDD->value();

 double dMax = 0.0;
 if (ui->comboBoxRefDose->currentText() == "Dref") dMax = inputData->dRef;
 if (ui->comboBoxRefDose->currentText() == "Dmax") dMax = inputData->dMax;
 if (ui->comboBoxRefDose->currentText() == "Pointwise") dMax = inputData->dMax;

 int cutOff = ui->spinBoxCutoff->value();
 float cutOffDose = dMax*cutOff/100.0;
 int nCut = 0;

 for (int i=0; i < inputData->nData; i++) {
  if (fabs(inputData->dd[i]) <= DD || fabs(inputData->dta[i]) <= DTA) {
   gammaText.sprintf("%7.3f", inputData->gamma[i]);
   pass++;  // Passed by DD or DTA
   //QTableWidgetItem *tItem = new QTableWidgetItem(tableDD,QTableWidgetItem::Never,gammaText);
   //ui->tableDD->setItem( i, 5, tItem );
   ui->tableDD->setItem( i, 5, new QTableWidgetItem(gammaText));
   ui->tableDD->item(i, 5)->setBackground(Qt::yellow);
    if (inputData->dm[i] <= cutOffDose) pass--;
  } else {
   if (inputData->gamma[i] >= 0.0 ) { // Gamma exists
    gammaText.sprintf("%7.3f", inputData->gamma[i]);
    if (fabs(inputData->gamma[i]) > 1) {  // Failed by Gamma
     //MyTableItem *tItem = new MyTableItem(tableDD,Q3TableItem::Never,gammaText);
     //ui->tableDD->setItem( i, 5, tItem );
     ui->tableDD->setItem( i, 5, new QTableWidgetItem(gammaText));
    } else {
     pass++; // Passed by Gamma
     //Q3TableItem *tItem = new Q3TableItem(tableDD,Q3TableItem::Never,gammaText);
     //ui->tableDD->setItem( i, 5, tItem );
     ui->tableDD->setItem( i, 5, new QTableWidgetItem(gammaText));
      if (inputData->dm[i] <= cutOffDose) pass--;
    }
   }else{  //  No Gamma Available
   }
  }
  if (inputData->gamma[i] >= 0.0 ) { // Gamma exists
      if (fabs(inputData->gamma[i]) <= 1) {
          gPass++;
          if (inputData->dm[i] <= cutOffDose) gPass--;
      }
  }
  if (inputData->dm[i] <= cutOffDose) nCut++;
 }
 QString passText;
 passText.sprintf("%7.3f", (pass/(inputData->nData-nCut)*100) );
 ui->lineEditPass->setText(passText);
 QString gPassText;
 gPassText.sprintf("%7.3f", (gPass/(inputData->nData-nCut)*100) );
 ui->lineEditGPass->setText(gPassText);

 hideLow();
}

void GammaTable::showHide() {
 int DTA = ui->spinBoxDTA->value();
 int DD  = ui->spinBoxDD->value();
 for (int i=0; i < inputData->nData; i++) {
    if (fabs(inputData->dd[i]) <= DD || fabs(inputData->dta[i]) <= DTA) {
    if (ui->radioButtonHide->isChecked()) {
            ui->tableDD->hideRow(i);
        }
        else {
            ui->tableDD->showRow(i);
        }
    }
    else {
            ui->tableDD->showRow(i);
    }
 }
}

void GammaTable::hideLow() {
 showHide();
 int CutOff = ui->spinBoxCutoff->value();
 QString refDoseText = ui->lineEditRefDose->text();
 float refDose = 0.0;
 bool ok;
 if (refDoseText.contains("Pointwise")) refDose = inputData->dMax;
 else refDose = refDoseText.toFloat(&ok);
 float cutOffDose = refDose*CutOff/100.0;
 for (int i=0; i < inputData->nData; i++) {
    if (inputData->dm[i] <= cutOffDose) {
        ui->tableDD->hideRow(i);
    }
 }
}

void GammaTable::updateAll() {
 updateDTA();
 updateDoseDiff();
}

void GammaTable::reLoadData() {
 int nRows = inputData->nData;
 QString FNAME = ui->comboBoxFiles->currentText();
 loadData(FNAME);
 // Remove old data tails
 for (int i=nRows-1; i>=inputData->nData; i--) {
  ui->tableDD->removeRow(i);
 }
 ui->dataLabel->setText(makeInfo(FNAME));
}

QString GammaTable::makeInfo(QString FNAME) {

 QString subTitle = "";
 QString xFieldSize = "";
 QString yFieldSize = "";
 QString FSunit = "(mm<SUP>2</SUP>)";
 QString profileText = "";
 QString depth = "";
 QString depthText = " Depth (cm): ";
 QString SSDtext = "";
 QString SSD = "";

 // cout << "FNAME (Gamma Table) = " << FNAME << endl;

 if (FNAME.contains("diff/") != 0) {
    subTitle = "In-air (Geometric) Fitting";
    depthText = " SDD (mm) : ";
    depth = FNAME.section('.',1,1) + "0";
    if (FNAME.contains("diff/x") != 0) {
     xFieldSize = FNAME.section('.',0,0).section("MV",1,1).section('x',0,0);
     yFieldSize = FNAME.section('.',0,0).section("MV",1,1).section('x',1,1);
     profileText = "Cross scan";
    }
    if (FNAME.contains("diff/y") != 0) {
     xFieldSize = FNAME.section('.',0,0).section("MV",1,1).section('x',0,0);
     yFieldSize = FNAME.section('.',0,0).section("MV",1,1).section('x',1,1);
     profileText = "In-line scan";
    }
    if (FNAME.contains("diff/z") != 0) {
     xFieldSize = FNAME.section('.',0,0).section("MV",1,1).section('x',0,0);
     yFieldSize = FNAME.section('.',0,0).section("MV",1,1).section('x',1,1);
     profileText = "Axial scan";
     depthText = " Off-axis (mm): ";
     depth = "0";
    }
 }

 if (FNAME.contains("diff.dd") !=0) {
    subTitle = "In-water (Energy) Fitting";
    xFieldSize = "100";
    yFieldSize = "100";
    profileText = "Axial scan";
    depthText = " Off-axis (mm): ";
    depth = "0";
 }

 if (FNAME.contains("Comp") !=0) {
    subTitle = "In-water Verification Comparison";

    xFieldSize = FNAME.section('.',2,2).section('x',0,0);
    yFieldSize = FNAME.section('.',2,2).section('x',1,1);
    if (FNAME.contains(".xComp")) {
       profileText = "Cross scan";
       depthText = " Depth (mm): ";
       depth = FNAME.section('.',4,4);
       SSDtext = " SSD (mm): ";
       SSD = FNAME.section('.',3,3) + "0";
    }
    if (FNAME.contains(".yComp")) {
       depthText = " Depth (mm): ";
       depth = FNAME.section('.',4,4);
       profileText = "In-plane scan";
       SSDtext = " SSD (mm): ";
       SSD = FNAME.section('.',3,3) + "0";
    }
    if (FNAME.contains(".zComp")) {
      profileText = "Axial scan";
      depthText = " Off-axis (mm): ";
      depth = "0";
    }
 }

 if (FNAME.contains("_") !=0) {
    subTitle = "PB vs MC";

    if (FNAME.contains("_x")) {
       QString FieldSize = FNAME.section("_x",1,1).section('.',0,0);
       int strLength = FieldSize.length()/2;
       xFieldSize = FieldSize.left(strLength);
       yFieldSize = FieldSize.right(strLength);
       profileText = "Cross Profile";
       depthText = " Depth (mm): ";
       depth = FNAME.section('.',1,1);
    }
    if (FNAME.contains("_z")) {
       QString FieldSize = FNAME.section("_z",1,1).section('.',0,0);
       int strLength = FieldSize.length()/2;
       xFieldSize = FieldSize.left(strLength);
       yFieldSize = FieldSize.right(strLength);
       profileText = "In-line Profile";
       depthText = " Depth (mm): ";
       depth = FNAME.section('.',1,1);
    }
 }
 QString dataInfo = "<p align=center> <b>"
        + subTitle +
        + ":</b> "
        + readSettingString("Machine/Machine")
        + " <b>Field Size " + FSunit + ": </b>"
        + xFieldSize + "x" + yFieldSize
        + " <br><b>Profile: </b>" + profileText
        + " <b>" + depthText + "</b>" + depth
        + " <b>" + SSDtext + "</b>" + SSD
        + "</p>";
 return(dataInfo);
}


QString GammaTable::readSettingString(QString keyWord){
   QString keyValue="";
#ifdef XML

   QString HOME=getenv("HOME");
   cout << "xmlFile = " << HOME+"/.monacobct" << endl;
   QFile xmlFile(HOME+"/.monacobct");
   xmlFile.open(QIODevice::ReadOnly);

   QDomDocument docType("MonacoPath");
   docType.setContent(&xmlFile);
   xmlFile.close();

   QString mName="XVMC";
   QDomElement root = docType.documentElement();
   if (root.tagName() != mName) {
      cout << "Tag Name ("<<root.tagName()<<") is different from " << mName << endl;
      exit(-1);
   }

   QDomNode n = root.firstChild();

   while (!n.isNull()){
      QDomElement e = n.toElement();
      if(!e.isNull()) {
         if(e.tagName() == keyWord) keyValue = e.attribute("value", "");
      }
      n = n.nextSibling();
   }

#else
 // TODO
 QString HOSTNAME = getenv("HOSTNAME");
 QString HOST = HOSTNAME.section('.',0,0);
 QString Entry = "/"+HOST+"/" + keyWord;
 //QSettings settings;
 //settings.setPath("localhost", HOST);
 QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
 QSettings settings("settings.ini", QSettings::IniFormat);
 settings.setValue("localhost", HOST);


 settings.beginGroup("/"+HOST);
 // keyValue = settings.readEntry(Entry, "");
 settings.endGroup();
#endif
 return (keyValue);
}

void GammaTable::printTable(){
  if (!ui->tableDD) return;
#ifndef QT_NO_PRINTER
  QPrinter printer( QPrinter::HighResolution );
  printer.setFullPage(TRUE);
  printer.setPageSize(QPrinter::Letter);
  printer.setColorMode(QPrinter::Color);
  //Q3PaintDeviceMetrics screen(this);
  printer.setResolution(QPaintDevice::logicalDpiY());
  if (printer.isValid()) {
    // QPainter p(&printer);
    // Q3PaintDeviceMetrics metrics(p.device());

    QPainter painter;
    painter.begin(&printer);

    int dpix = QPaintDevice::logicalDpiX();
    int dpiy = QPaintDevice::logicalDpiY();
    const int margin = 50; // pt
    QRect body(margin * dpix / 72, margin * dpiy / 72,
               QPaintDevice::width() - margin * dpix / 72 * 2,
               QPaintDevice::height() - margin * dpiy / 72 * 2);
    QFont font("Trebuchet MS", 10);
    QString string;

  // Title
 // string += "<table width=\"100%\" border=0 cellspacing=0>\n";
 // string += "<tr><td align=\"center\"><b><font size=\"+2\">";
 // string += "Dose Difference and DTA Table";
 // string += "</font></b></td></tr></table><hr>\n";

  // Machine Name
  string += "<H2 align=center";
  string += "<b>XiO/Monaco Beam Fitting Analysis Table</b>";
  string += "</H2>\n";
  string += ui->dataLabel->text();
  string += "<hr>\n";

  // Information
  string += "<table width=\"100%\" border=0 cellspacing=0>\n";
  string += "<tr>";
  string += "<td> DD(%) =  </td> <td>";
    QString ddText;
    ddText.sprintf("%d", ui->spinBoxDD->value());
  string += ddText;
  string += "</td>";

  string += "<td> DD Pass(%) =  </td> <td>";
    string += ui->lineEditDdPass->text();
  string += "</td>";

    if (ui->comboBoxRefDose->currentText() == "Dref") {
       string += "<td> Ref Dose (Gy) = </td> <td>";
       string += ui->lineEditRefDose->text();
       string += " (Dref) ";
    } else if (ui->comboBoxRefDose->currentText() == "Dmax") {
       string += "<td> Ref Dose (Gy) = </td> <td>";
       string += ui->lineEditRefDose->text();
       string += " (Dmax) ";
    } else {
       string += "<td> DD = (C-M)/M (%) </td> <td>";
       string += " (Pointwise) ";
    }
    string += "</td>";
    string += "</tr>\n";

  string += "<tr>";
  string += "<td> DTA(mm) =  </td> <td>";
    QString dtaText;
    dtaText.sprintf("%d", ui->spinBoxDTA->value());
  string += dtaText;
  string += "</td>";

  string += "<td> DTA Pass(%) =  </td> <td>";
    string += ui->lineEditDtaPass->text();
  string += "</td>";

  string += "<td> DD or DTA Pass(%) =  </td> <td>";
    string += ui->lineEditPass->text();
  string += "</td>";
  string += "</tr>\n";
  string += "</table><hr>\n";

    int numCols = ui->tableDD->columnCount();
  string += "<table width=\"100%\" border=0 cellspacing=0 cellpadding=0>\n";
  string += "<font size=\"8 pt\">";
  string += " <tr>\n ";

  string += " <td align=center width=\"8%\"> ";
  string += " ";
  string += "</td>\n";

    for (int col = 0; col != numCols; col++){
    string += " <td align=center width=\"15%\" bgcolor=#EEEEEE> ";
    //string += ui->tableDD->horizontalHeader()->label(col);
    string += ui->tableDD->horizontalHeaderItem(col)->text();
    string += " </td>\n ";
    }
    string += " </tr>\n ";

    int numRows = ui->tableDD->rowCount();
    for (int row = 0; row != numRows; row++){
         if(!ui->tableDD->isRowHidden(row)){
        string += " <tr>\n ";
        string += " <td align=center bgcolor=#EEEEEE height=8> ";
        QString rowNumText;
        rowNumText.sprintf("%d", row+1);
        string += rowNumText;
        string += "</td>\n";
        for (int col = 0; col != numCols; col++){
            QString bgcolor = "#FFFFFF";
            if (col == 3 && fabs(inputData->dd[row]) > ui->spinBoxDD->value()) bgcolor = "yellow";
            if (col == 4 && fabs(inputData->dta[row]) > ui->spinBoxDTA->value()) bgcolor = "yellow";
            if (col == 5 && ui->tableDD->item(row,col)->text().contains("passed") == 0) bgcolor = "yellow";
            string += " <td align=right height=8 bgcolor=" + bgcolor + "> ";
            string += ui->tableDD->item(row,col)->text();
            string += " </td>\n ";
        }
        string += " </tr>\n ";
       }
    }
    string += " </font></table>\n ";

    QDateTime dateTime;
    QString runningTitle = ui->dataLabel->text();
    runningTitle = runningTitle.section("</b>",1,1).section("<b>",0,0)
                 + " " + runningTitle.section("</b>",2,2).section("<br>",0,0)
                 + " " + runningTitle.section("</b>",3,3).section("<b>",0,0)
                 + " " + runningTitle.section("</b>",4,4).section("<b>",0,0)
                 + " " + dateTime.currentDateTime().toString()
                 + " Page:";
    // Q3SimpleRichText richText(string, font, "", 0, 0, body.height());
    QTextDocument richText;
    richText.setHtml(string);
    // richText.setWidth(&p, body.width());
    richText.setTextWidth(body.width());
    QRect view(body);
    int page = 1;
    do {
      //richText.draw(&p, body.left(), body.top(), view, colorGroup());
      //view.moveBy(0, body.height());
        view.moveTop(body.height());
      // p.translate(0 , -body.height());
      painter.translate(0 , -body.height());
      painter.setFont(font);
      painter.drawText(view.right() - painter.fontMetrics().width(QString::number(page)+runningTitle),
                 view.bottom() + painter.fontMetrics().ascent() + 5,
                 runningTitle + " " + QString::number(page));
      if (view.top() >= richText.size().height()) break;
      printer.newPage();
      page++;
    } while (TRUE);
    painter.end();
  }
 }
#endif
