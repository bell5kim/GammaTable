#ifndef GAMMATABLE_H
#define GAMMATABLE_H

#include <QDialog>

namespace Ui {
class GammaTable;
}

class GammaTable : public QDialog
{
    Q_OBJECT

public:
    explicit GammaTable(QWidget *parent = 0);
    ~GammaTable();

    // ----- from QT3 gammatable.h
    void init();
    virtual QString makeInfo( QString FNAME );
    virtual QString readSettingString( QString keyWord );

public slots:
    virtual void initLoad( char * fname );
    virtual void loadData( QString FNAME );
    virtual void updateDoseDiff();
    virtual void calcDTA();
    virtual void updateDTA();
    virtual void calcGamma();
    virtual void updateGamma();
    virtual void showHide();
    virtual void hideLow();
    virtual void updateAll();
    virtual void reLoadData();
    virtual void printTable();

private:
    Ui::GammaTable *ui;
};

#endif // GAMMATABLE_H
