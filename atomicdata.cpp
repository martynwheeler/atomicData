#include "atomicdata.h"
#include "ui_atomicdata.h"

//#define DEBUG

AtomicData::AtomicData(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AtomicData)
{
    ui->setupUi(this);

    /* make debug tab invisible if in debug mode*/
    #ifndef DEBUG
    ui->tabWidget->removeTab(3);
    ui->checkBox->setChecked(false);
    Atom::useAccurate_ = ui->checkBox->isChecked();
    //ui->checkBox->hide();
    #endif

    /* set up labels */
    ui->label_electronCharge->setText("e = " + QString::number(Atom().getElectronCharge(), 'g', 12) + " C");
    ui->label_speedofLight->setText("c = " + QString::number(Atom().getSpeedofLight(), 'g', 12) + " m/s");
    ui->label_amu->setText("u = " + QString::number(Atom().getAtomicMassUnit(), 'g', 12) + " kg");
    ui->label_electronMass->setText("me = " + QString::number(Atom().getElectronMass(), 'g', 12) + " u");
    ui->label_protonMass->setText("mp = " + QString::number(Atom().getProtonMass(), 'g', 12) + " u");
    ui->label_neutronMass->setText("mn = " + QString::number(Atom().getNeutronMass(), 'g', 12) + " u");

    /* set up table for displaying all of the nuclear data */
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setColumnCount(8);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Neutons"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Protons"));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("A"));
    ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Symbol"));
    ui->tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Binding Energy"));
    ui->tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem("Uncertainty"));
    ui->tableWidget->setHorizontalHeaderItem(6, new QTableWidgetItem("Atomic Mass"));
    ui->tableWidget->setHorizontalHeaderItem(7, new QTableWidgetItem("Uncertainty"));

    /* try to open file to read */
    std::ifstream ifile;
    ifile.open("nuclear_data.csv");

    /* check if file exists and parse or download from server */
    if(ifile) {
        /* parse the data from the csv file */
        processDataFromFile(ifile);
        ifile.close();
        /* plot a graph of the data */
        plotNuclearData(ui->customPlot);
    } else {
        /* file does not exist - connect to server and download data */
        QNetworkAccessManager *mNetworkManager = new QNetworkAccessManager(this);
        connect(mNetworkManager, &QNetworkAccessManager::finished, this, &AtomicData::onNetworkReply);
        /* make a request to the server */
        mNetworkManager->get(QNetworkRequest(QUrl("https://www-nds.iaea.org/amdc/ame2016/mass16.txt")));
    }
}

/* destructor */
AtomicData::~AtomicData()
{
    delete ui;
}

/* function called if no input data exists to get data from server */
void AtomicData::onNetworkReply(QNetworkReply* reply)
{
    const int RESPONSE_OK = 200;
    const int RESPONSE_ERROR = 404;
    const int RESPONSE_BAD_REQUEST = 400;

    if(reply->error() == QNetworkReply::NoError)
    {
        int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
        switch(httpstatuscode)
        {
        case RESPONSE_OK:
            // Request accepted and reply has been sent
            if (reply->isReadable()){
                /* replyString now contains the file */
                QString replyString = QString::fromUtf8(reply->readAll().data());

                /* process the data */
                processDataFromServer(replyString);

                /* plot a graph */
                plotNuclearData(ui->customPlot);
            }
            break;
        case RESPONSE_ERROR:
            // Process a 404 error
            break;
        case RESPONSE_BAD_REQUEST:
            // Process a 400 error
            break;
        default:
            break;
        }
    }
    reply->deleteLater();
}

/* function called process the file from server */
void AtomicData::processDataFromServer(QString &dataFromServer){
    /* split the data into rows */
    QStringList l = dataFromServer.split('\n');
    int numberofRows = l.count() - 1;

    /* set the number of nuclei */
    this->numberOfNuclei_ = numberofRows - 39;

    /* define a variable to hold substring */
    QString subStringValue;

    /* csvString to hold data */
    std::string csvString = "";

    /* data starts on row 39 */
    for (int row = 39; row < numberofRows ; row++) {
        /* insert a row to table */
        ui->tableWidget->insertRow( ui->tableWidget->rowCount() );

        /* set up rowConter for table */
        int rowCounter = row - 39;

        /* get the number of neutrons */
        subStringValue = l[row].mid(4,5).trimmed();
        int neutrons = subStringValue.toInt();
        csvString += subStringValue.toStdString() + ",";
        ui->tableWidget->setItem(rowCounter, 0, new QTableWidgetItem(subStringValue));

        /* get the number of protons */
        subStringValue = l[row].mid(9,5).trimmed();
        int protons = subStringValue.toInt();
        csvString += subStringValue.toStdString() + ",";
        ui->tableWidget->setItem(rowCounter, 1, new QTableWidgetItem(subStringValue));

        /* get the number of nucleons */
        subStringValue = l[row].mid(14,5).trimmed();
        int nucleons = subStringValue.toInt();
        csvString += subStringValue.toStdString() + ",";
        ui->tableWidget->setItem(rowCounter, 2, new QTableWidgetItem(subStringValue));

        /* get the element name */
        subStringValue = l[row].mid(20,3).trimmed();
        std::string element = subStringValue.toStdString();
        csvString += element + ",";
        ui->tableWidget->setItem(rowCounter, 3, new QTableWidgetItem(subStringValue));

        /* get the binding energy */
        subStringValue = l[row].mid(53,11).trimmed();
        if (subStringValue.contains("#")){
            subStringValue.replace(subStringValue.indexOf("#"), 2, ".0");
        }
        double bindingEnergy = subStringValue.toDouble();
        csvString += subStringValue.toStdString() + ",";
        ui->tableWidget->setItem(rowCounter, 4, new QTableWidgetItem(subStringValue));

        /* get the binding energy uncertainty */
        subStringValue = l[row].mid(64,9).trimmed();
        if (subStringValue.contains("#")){
            subStringValue.replace(subStringValue.indexOf("#"), 2, ".0");
        }
        double bindingEnergyUncertainty = subStringValue.toDouble();
        csvString += subStringValue.toStdString() + ",";
        ui->tableWidget->setItem(rowCounter, 5, new QTableWidgetItem(subStringValue));

        /* get the atomic mass */
        subStringValue = l[row].mid(96,3).trimmed();
        subStringValue += l[row].mid(100,12).trimmed();
        if (subStringValue.contains("#")){
            subStringValue.replace(subStringValue.indexOf("#"), 2, ".0");
        }
        double atomicMass = subStringValue.toDouble();
        csvString += subStringValue.toStdString() + ",";
        ui->tableWidget->setItem(rowCounter, 6, new QTableWidgetItem(subStringValue));

        /* get the atomic mass uncertainty */
        subStringValue = l[row].mid(112,11).trimmed();
        if (subStringValue.contains("#")){
            subStringValue.replace(subStringValue.indexOf("#"), 2, ".0");
        }
        double atomicMassUncertainty = subStringValue.toDouble();
        csvString += subStringValue.toStdString() + "\n";
        ui->tableWidget->setItem(rowCounter, 7, new QTableWidgetItem(subStringValue));

        /* create an atom object array element */
        this->atoms_[rowCounter] = Atom(neutrons, protons, nucleons, element, bindingEnergy, bindingEnergyUncertainty, atomicMass, atomicMassUncertainty);

    }
    /* define name for output file */
    std::ofstream out("nuclear_data.csv");
    out << csvString;
    out.close();
}

/* function called if input data exists to get data from file */
void AtomicData::processDataFromFile(std::ifstream &ifile){
    /* Use a while loop together with the getline() function to read the file line by line */
    std::string line;
    int rowCounter = 0;
    while (getline (ifile, line)) {
        /* insert a new row into the table */
        ui->tableWidget->insertRow( ui->tableWidget->rowCount() );
        std::istringstream s(line);
        std::string field;
        int count = 0;
        /* parse the line into fields */
        while (getline(s, field, ',')) {
            ui->tableWidget->setItem(rowCounter, count, new QTableWidgetItem(QString::fromStdString(field)));
            count++;
        }
        /* create an atom object array element */
        this->atoms_[rowCounter] = Atom((ui->tableWidget->item(rowCounter, 0)->text()).toInt(),
                                        (ui->tableWidget->item(rowCounter, 1)->text()).toInt(),
                                        (ui->tableWidget->item(rowCounter, 2)->text()).toInt(),
                                        (ui->tableWidget->item(rowCounter, 3)->text()).toStdString(),
                                        (ui->tableWidget->item(rowCounter, 4)->text()).toDouble(),
                                        (ui->tableWidget->item(rowCounter, 5)->text()).toDouble(),
                                        (ui->tableWidget->item(rowCounter, 6)->text()).toDouble(),
                                        (ui->tableWidget->item(rowCounter, 7)->text()).toDouble());
        rowCounter++;
    }
    /* set the number of nuclei */
    this->numberOfNuclei_ = rowCounter;
}

/* plot the data */
void AtomicData::plotNuclearData(QCustomPlot *customPlot)
{
  // generate some data:
  QVector<double> x(this->numberOfNuclei_), y1(this->numberOfNuclei_), y2(this->numberOfNuclei_); // initialize with entries 0..this->numberOfNuclei
  getMaxEnergies(x, y1, y2);
  //getAllEnergies(x, y1, y2);

  // create graph and assign data to it:
  customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
  customPlot->legend->setVisible(true);
  customPlot->setInteraction(QCP::iRangeDrag, true);
  customPlot->setInteraction(QCP::iRangeZoom, true);
  QFont legendFont = font();  // start out with MainWindow's font..
  legendFont.setPointSize(9); // and make a bit smaller for legend
  customPlot->legend->setFont(legendFont);
  customPlot->legend->setBrush(QBrush(QColor(255,255,255,230)));
  // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
  customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis);
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setData(x, y1);
  customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
  customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
  customPlot->graph(0)->setName("Binding Energy per Nucleon");

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2);
  customPlot->graph(1)->setPen(QPen(Qt::red));
  customPlot->graph(1)->setData(x, y2);
  customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
  customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
  customPlot->graph(1)->setName("Total Binding Energy");

  // activate right axes, which is invisible by default:
  customPlot->yAxis2->setVisible(true);
  // give the axes some labels:
  customPlot->xAxis->setLabel("Nucleon Number (A)");
  customPlot->yAxis->setLabel("Binding Energy per Nucleon / MeV");
  customPlot->yAxis2->setLabel("Total Binding Energy / MeV");
  // set axes ranges, so we see all data:
  customPlot->xAxis->setRange(0, 300);
  customPlot->yAxis->setRange(0, 10);
  customPlot->yAxis2->setRange(0, 2500);
}

/* find the nucleus specified by nucleon number and element name */
int AtomicData::findNucleus(const int nucleonNumber, const int protonNumber){
    /* set retun variable to -1 in case of not found */
    int foundNucleus = -1;

    /* declare an array to hold nuclei with correct nucleon number */
    int foundNuclei[this->numberOfNuclei_];

    /* counter to hold the number of neclei with correct nucleon number */
    int counter = 0;

    /* search atoms for nuclei with correct nucleon number */
    for (int i = 0; i < this->numberOfNuclei_; i++){
        /* skip through < number */
        if (atoms_[i].getNucleons() < nucleonNumber) continue;
        if (atoms_[i].getNucleons() == nucleonNumber){
            //add the nucleus to the found array
            foundNuclei[counter] = i;
            counter++;
        } else break;
    }

    /* search the subarray for correct nucleus */
    if (counter > 0) {
        for (int i = 0; i < counter; i++){
            if (this->atoms_[foundNuclei[i]].getProtons() == protonNumber){
                foundNucleus = foundNuclei[i];
            }
        }
    }

    /* return the array index of the found nucleus */
    return foundNucleus;
}

/* calculate the data for selected nucleus */
void AtomicData::on_pushButtonCalculate_clicked()
{
    /* use the selected values of A and Z to find a nucleus */
    int found = this->findNucleus(ui->spinBoxNucleonNumber->value(), ui->spinBoxProtonNumber->value());

    /* set up table to view properties */
    ui->tableWidgetOutput->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetOutput->setColumnCount(2);
    ui->tableWidgetOutput->setRowCount(10);
    ui->tableWidgetOutput->setHorizontalHeaderItem(0, new QTableWidgetItem("Property"));
    ui->tableWidgetOutput->setColumnWidth(0, ui->tableWidgetOutput->width() * 0.6);
    ui->tableWidgetOutput->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
    ui->tableWidgetOutput->setItem(0, 0, new QTableWidgetItem("Element"));

    /* display data if found */
    if (found >= 0){
        Atom::useAccurate_ = ui->checkBox->isChecked();
        ui->tableWidgetOutput->setItem(0, 1, new QTableWidgetItem(QString::fromStdString(this->atoms_[found].getElement())));
        ui->tableWidgetOutput->setItem(1, 0, new QTableWidgetItem("Protons"));
        ui->tableWidgetOutput->setItem(1, 1, new QTableWidgetItem(QString::number(this->atoms_[found].getProtons())));
        ui->tableWidgetOutput->setItem(2, 0, new QTableWidgetItem("Neutrons"));
        ui->tableWidgetOutput->setItem(2, 1, new QTableWidgetItem(QString::number(this->atoms_[found].getNeutrons())));
        ui->tableWidgetOutput->setItem(3, 0, new QTableWidgetItem("Atomic Mass / amu"));
        ui->tableWidgetOutput->setItem(3, 1, new QTableWidgetItem(QString::number(this->atoms_[found].getAtomicMass(), 'g', 8)));
        ui->tableWidgetOutput->setItem(4, 0, new QTableWidgetItem("Nuclear Mass / amu"));
        ui->tableWidgetOutput->setItem(4, 1, new QTableWidgetItem(QString::number(this->atoms_[found].calcNuclearMass(), 'g', 8)));
        ui->tableWidgetOutput->setItem(5, 0, new QTableWidgetItem("Nuclear Mass Defect / amu"));
        ui->tableWidgetOutput->setItem(5, 1, new QTableWidgetItem(QString::number(this->atoms_[found].calcMassDefectamu(), 'g', 8)));
        ui->tableWidgetOutput->setItem(6, 0, new QTableWidgetItem("Nuclear Mass Defect / kg"));
        ui->tableWidgetOutput->setItem(6, 1, new QTableWidgetItem(QString::number(this->atoms_[found].calcMassDefectkg(), 'g', 8)));
        ui->tableWidgetOutput->setItem(7, 0, new QTableWidgetItem("Nuclear Binding Energy / J"));
        ui->tableWidgetOutput->setItem(7, 1, new QTableWidgetItem(QString::number(this->atoms_[found].calcBindingEnergyJ(), 'g', 8)));
        ui->tableWidgetOutput->setItem(8, 0, new QTableWidgetItem("Nuclear Binding Energy / MeV"));
        ui->tableWidgetOutput->setItem(8, 1, new QTableWidgetItem(QString::number(this->atoms_[found].calcBindingEnergykeV()/1.0e3, 'g', 8)));
        ui->tableWidgetOutput->setItem(9, 0, new QTableWidgetItem("Binding Energy per Nucleon/ MeV"));
        ui->tableWidgetOutput->setItem(9, 1, new QTableWidgetItem(QString::number(this->atoms_[found].calcBindingEnergyperNucleonkeV()/1.0e3, 'g', 8)));
    } else {
        ui->tableWidgetOutput->setItem(0, 1, new QTableWidgetItem("Not Found"));
    }
}

void AtomicData::getMaxEnergies(QVector<double> &x, QVector<double> &y1, QVector<double> &y2){
    int maxNucleonNumber = this->atoms_[this->numberOfNuclei_ - 1].getNucleons();
    int counter = 0;
    for (int i = 0; i < maxNucleonNumber ; i++) {
        int nucleonNumber = i + 1;
        double maxEnergy = 0;
        while (atoms_[counter].getNucleons() == nucleonNumber){
            double newEnergy = atoms_[counter].getBindingEnergy();
            if (newEnergy > maxEnergy){
                maxEnergy = newEnergy;
            }
            counter++;
        }
        x[i] = nucleonNumber;
        y1[i] = maxEnergy / 1.0e3;
        y2[i] = maxEnergy * nucleonNumber / 1.0e3;
    }
}

void AtomicData::getAllEnergies(QVector<double> &x, QVector<double> &y1, QVector<double> &y2){
    for (int i = 0; i < this->numberOfNuclei_; ++i){
      x[i] = atoms_[i].getNucleons();             // Nucleon Number
      y1[i] = atoms_[i].getBindingEnergy()/1e3;   // Binding Energy / Nucleon
      y2[i] = y1[i] * x[i];                       // Total Binding Energy
    }
}

/*
void AtomicData::on_pushButton_clicked()
{
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_2->setColumnCount(8);
    ui->tableWidget_2->setRowCount(ui->tableWidget->rowCount());
    ui->tableWidget_2->setHorizontalHeaderItem(0, new QTableWidgetItem("Element"));
    ui->tableWidget_2->setHorizontalHeaderItem(1, new QTableWidgetItem("A"));
    ui->tableWidget_2->setHorizontalHeaderItem(2, new QTableWidgetItem("Z"));
    ui->tableWidget_2->setHorizontalHeaderItem(3, new QTableWidgetItem("Binding Energy"));
    ui->tableWidget_2->setHorizontalHeaderItem(4, new QTableWidgetItem("Binding Energy Calc"));
    ui->tableWidget_2->setHorizontalHeaderItem(5, new QTableWidgetItem("Obs - Calc"));

    for (int i = 0; i < ui->tableWidget->rowCount(); i++){
        ui->tableWidget_2->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(this->atoms_[i].getElement())));
        ui->tableWidget_2->setItem(i, 1, new QTableWidgetItem(QString::number(this->atoms_[i].getNucleons())));
        ui->tableWidget_2->setItem(i, 2, new QTableWidgetItem(QString::number(this->atoms_[i].getProtons())));
        ui->tableWidget_2->setItem(i, 3, new QTableWidgetItem(QString::number(this->atoms_[i].calcBindingEnergyJ(), 'g', 8)));
        ui->tableWidget_2->setItem(i, 4, new QTableWidgetItem(QString::number(this->atoms_[i].calcBindingEnergykeV() * 1.0000000983 * 1.602176634e-16, 'g', 8)));
        ui->tableWidget_2->setItem(i, 5, new QTableWidgetItem(QString::number(this->atoms_[i].calcBindingEnergyJ() - this->atoms_[i].calcBindingEnergykeV() * 1.0000000983 * 1.602176634e-16, 'g', 8)));
    }


//    double test = this->atoms_[7].calcBindingEnergyperNucleonkeV();
//    qDebug() << QString("%1").arg(test, 0, 'e', 9);
//    qDebug() << printf("%11.3f \n", test);
}
*/

void AtomicData::on_checkBox_stateChanged(int /* arg1 */)
{
    /* set up labels */
    Atom::useAccurate_ = ui->checkBox->isChecked();
    ui->label_electronCharge->setText("e = " + QString::number(Atom().getElectronCharge(), 'g', 12) + " C");
    ui->label_speedofLight->setText("c = " + QString::number(Atom().getSpeedofLight(), 'g', 12) + " m/s");
    ui->label_amu->setText("u = " + QString::number(Atom().getAtomicMassUnit(), 'g', 12) + " kg");
    ui->label_electronMass->setText("me = " + QString::number(Atom().getElectronMass(), 'g', 12) + " u");
    ui->label_protonMass->setText("mp = " + QString::number(Atom().getProtonMass(), 'g', 12) + " u");
    ui->label_neutronMass->setText("mn = " + QString::number(Atom().getNeutronMass(), 'g', 12) + " u");
}
