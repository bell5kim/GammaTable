#ifndef USER_H
#define USER_H

#include <QString>
#define MAXDATA 5000

class InputData {
    public:
       void err_Exit(char *location, char *reason, int exit_code);
        void getInputFile(QString fname);

        double x[MAXDATA];    // x position
        double y[MAXDATA];    // y position
        double z[MAXDATA];    // z position
        double dc[MAXDATA];	  // calculated dair
        double dm[MAXDATA];	  // measured dair
        double dd[MAXDATA];	  // (dc-dm)/dm*100
        double dta[MAXDATA];  // DTA
        double gamma[MAXDATA];  // Gamma
        int    nData;         // number of data points
        double dMax;
        double dRef;

};

#endif // USER_H

