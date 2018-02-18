#include "gammatable.h"
#include <QApplication>

#include <cstdio>

void usage() {
  printf("\nUSAGE: %s -i tablefile\n", "GammaTable.exe");
  printf("\nUSAGE: %s -i tablefile1:tablefile2:tablefile3\n", "GammaTable.exe");
  printf("\n");
}

int main(int argc, char *argv[])
{
    // Argument Checking --------
    char *fName="";

    if (argc == 1) {
        usage();
        return (0);
    }
    for(int iArg=0; iArg < argc; iArg++){
        if(iArg < argc-1){
        if( strcmp(argv[iArg],"-i") == 0 || strcmp(argv[iArg],"-table") == 0) {
                iArg++;
                fName = argv[iArg];
        }
        }
        if(strcmp("-help", argv[iArg]) == 0 || strcmp("-h", argv[iArg]) == 0 ) {
         usage();
         return(0);
        }
    }

    QApplication a(argc, argv);
    // GammaTable w;
    // w.show();

    GammaTable *gTable = new GammaTable;
    gTable->initLoad(fName);
    gTable->show();

    return a.exec();
}
