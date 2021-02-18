#ifndef ATOMICDATA_H
#define ATOMICDATA_H

#include <QMainWindow>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include "atom.h"
#include "qcustomplot.h"
#include <fstream>
#include <sstream>

QT_BEGIN_NAMESPACE
namespace Ui { class AtomicData; }
QT_END_NAMESPACE

class AtomicData : public QMainWindow
{
    Q_OBJECT

public:
    AtomicData(QWidget *parent = nullptr);
    ~AtomicData();

private slots:
    int findNucleus(const int nucleonNumber, const int protonNumber);
    void onNetworkReply(QNetworkReply* reply);
    void on_pushButtonCalculate_clicked();

    void on_checkBox_stateChanged(int arg1);

private:
    Ui::AtomicData *ui;

    /* variable to store atom data */
    static constexpr int maxAtoms_ = 3500;
    Atom atoms_[maxAtoms_];
    int numberOfNuclei_;

    /* private functions */
    void plotNuclearData(QCustomPlot *customPlot);
    void getMaxEnergies(QVector<double> &x, QVector<double> &y1, QVector<double> &y2);
    void getAllEnergies(QVector<double> &x, QVector<double> &y1, QVector<double> &y2);
    void processDataFromServer(QString &dataFromServer);
    void processDataFromFile(std::ifstream &ifile);
};
#endif // ATOMICDATA_H
