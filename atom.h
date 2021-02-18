#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <math.h>

class Atom
{
private:
    /* Private class members */
    int neutrons_;
    int protons_;
    int nucleons_;
    std::string element_;
    double bindingEnergy_;
    double bindingEnergyUncertainty_;
    double atomicMass_;
    double atomicMassUncertainty_;

    /* accurate numerical constants */
    static constexpr double amu_ = 1.660539040e-27;
    static constexpr double electronMass_ = 5.48579909070e-4;
    static constexpr double protonMass_ = 1.00727646693;
    static constexpr double hydrogenMass_ = 1.00782503224;
    static constexpr double neutronMass_ = 1.00866491582;
    static constexpr double speedofLight_ = 2.99792458e8;
    static constexpr double electronCharge_ = 1.0000000983 * 1.602176634e-19;
    static constexpr double amutokeV_() { return 931494.0038; };

    /* a-level numerical constants */
    static constexpr double amuA_ = 1.661e-27;
    static constexpr double electronMassA_ = 5.485e-4;
    static constexpr double protonMassA_ = 1.0072;
    static constexpr double neutronMassA_ = 1.0084;
    static constexpr double speedofLightA_ = 3.00e8;
    static constexpr double electronChargeA_ = 1.60e-19;
    static constexpr double amutokeVA_() { return (amuA_ * std::pow(speedofLightA_, 2) / electronChargeA_ * 1e-3); };

public:
    /* Atom default constructor */
    Atom();

    /* Atom constructor with data */
    Atom(int neutrons, int protons, int nucleons, std::string element, double bindingEnergy, double bindingEnergyUncertainty, double atomicMass, double atomicMassUncertainty);

    /* flag to determine which set of constants to use */
    static bool useAccurate_;

    /* getters */
    int getNeutrons() { return this->neutrons_; }
    int getProtons() { return this->protons_; }
    int getNucleons()  { return this->nucleons_; }
    std::string getElement()  { return this->element_; }
    double getBindingEnergy() { return this->bindingEnergy_; }
    double getBindingEnergyUncertainty() { return this->bindingEnergyUncertainty_; }
    double getAtomicMass() { return this->atomicMass_; }
    double getAtomicMassUncertainty() { return this->atomicMassUncertainty_; }
    double getElectronCharge();
    double getAtomicMassUnit();
    double getSpeedofLight();
    double getElectronMass();
    double getProtonMass();
    double getNeutronMass();

    /* functions to calculate nuclear properties */
    double calcNuclearMass();
    double calcMassDefectamuAlt();
    double calcMassDefectamu();
    double calcMassDefectkg();
    double calcBindingEnergyJ();
    double calcBindingEnergykeV();
    double calcBindingEnergyperNucleonkeV();
};


#endif // ATOM_H
