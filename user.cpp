#include "user.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

// error handler
void InputData::err_Exit(char *location, char *reason, int exit_code)
{
   cerr << endl;
   cerr << "RUN-TIME ERROR in " << location << endl;
   cerr << "Reason: " << reason << "!" << endl;
   cerr << "Exiting to system..." << endl;
   cerr << endl;
   exit(exit_code);
}

void InputData::getInputFile (QString fname) {

    QString FNAME;
    // if (fname.find('/',0,true) != -1) {
    if (fname.contains('/')) {
        FNAME = fname.section('/',1,1);
    } else {
        FNAME = fname;
    }
    // cout << "FNAME = " << FNAME << endl;


    if (strncmp(FNAME.toStdString().c_str(),"_",1) == 0) {
        QString sFNAME = FNAME;
        sFNAME.prepend("PBComm/scaled");
        QString fFNAME = FNAME;
        fFNAME.prepend("PBComm/fitted");
       // cout << "sFNAME = " << sFNAME << endl;
       // cout << "fFNAME = " << fFNAME << endl;

        ifstream inp1_file;
    inp1_file.open(sFNAME.toStdString().c_str(),ios::in);
    if (inp1_file.bad()) err_Exit("GammaTable::getInputFile","cannot open input file1",8);

        int i = 0;
    while (!inp1_file.eof()) // Scaled Profile
    {
        char line[256] = "";  // lines to read from file
        inp1_file.getline(line,sizeof(line));
        istringstream line_stream(line);
            line_stream >> x[i] >> dm[i];
            x[i] /= 10.0;
            i++;
        }
        nData = i-1;
        // close file
    inp1_file.close();

        ifstream inp2_file;
    inp2_file.open(fFNAME.toStdString().c_str(),ios::in);
    if (inp2_file.bad()) err_Exit("GammaTable::getInputFile","cannot open input file1",8);

        // cout << "fFNAME = " << fFNAME << endl;

        i = 0;
        double xfit[5000];
        double dcfit[5000];
    while (!inp2_file.eof())  // Fitted Profile
    {
        char line[256] = "";  // lines to read from file
        inp2_file.getline(line,sizeof(line));
        istringstream line_stream(line);

            line_stream >> xfit[i] >> dcfit[i];
            xfit[i] /= 10.0;
            i++;
        }
        int nFit = i-1;
        // close file
    inp2_file.close();

        for (int im = 0; im < nData; im++) {
            for (int ifit = 0; ifit < nFit-1; ifit++) {
                if ((xfit[ifit]-x[im])*(xfit[ifit+1]-x[im]) <= 0.0) {
                    dc[im] = (dcfit[ifit]*fabs(xfit[ifit+1]-x[im])
                            +dcfit[ifit+1]*fabs(xfit[ifit]-x[im]))
                             / fabs(xfit[ifit+1]-xfit[ifit]);
                    break;
                }
            }
        }

    } else {
        ifstream inp_file;
    inp_file.open(fname.toStdString().c_str(),ios::in);
    if (inp_file.bad()) err_Exit("GammaTable::getInputFile","cannot open input file",8);

        int i = 0;
    while (!inp_file.eof())
    {
        char line[256] = "";  // lines to read from file
        inp_file.getline(line,sizeof(line));
        istringstream line_stream(line);

            if (strncmp(FNAME.toStdString().c_str(),"x",1) == 0 ||
                 strncmp(FNAME.toStdString().c_str(),"y",1) == 0 ||
                 strncmp(FNAME.toStdString().c_str(),"z",1) == 0) {
             line_stream >> x[i] >> y[i] >> z[i] >> dc[i] >> dm[i] >> dd[i];
            }
            if (strncmp(FNAME.toStdString().c_str(),"y",1) == 0) x[i] = y[i];
            if (strncmp(FNAME.toStdString().c_str(),"z",1) == 0) x[i] = z[i];

            if (strncmp(FNAME.toStdString().c_str(),"diff.dd",7) == 0) {
                line_stream >> x[i] >> dc[i] >> dm[i] >> dd[i];
            } else {
                line_stream >> x[i] >> dc[i] >> dm[i];
            }
            i++;
        }

        nData = i-1;
        // close file
    inp_file.close();
   }
}
