#include "atom.h"

/* default empty contructor - needed to initialise array of objects */
Atom::Atom()
{
}

/* contructor which initialises atom with values */
Atom::Atom(int neutrons, int protons, int nucleons, std::string element, double bindingEnergy, double bindingEnergyUncertainty, double atomicMass, double atomicMassUncertainty)
{
    this->neutrons_ = neutrons;
    this->protons_ = protons;
    this->nucleons_ = nucleons;
    this->element_ = element;
    this->bindingEnergy_ = bindingEnergy;
    this->bindingEnergyUncertainty_ = bindingEnergyUncertainty;
    this->atomicMass_ = atomicMass * 1.0e-6;                        // convert to u
    this->atomicMassUncertainty_ = atomicMassUncertainty * 1.0e-6;  // convert to u
}

/* set default as accuarate */
bool Atom::useAccurate_ = true;

/* returns the nuclear mass given the atomic mass and proton number */
double Atom::calcNuclearMass(){
    if (Atom::useAccurate_){
        return (this->atomicMass_ - (this->protons_ * this->electronMass_));
//        return (this->atomicMass_ - (this->protons_ * this->electronMass_) + (14.4381 * std::pow(this->protons_, 2.39) + 1.55468e-6 * std::pow(this->protons_, 5.35)) * 1e-3 / this->amutokeV_());
    } else {
        return (this->atomicMass_ - (this->protons_ * this->electronMassA_));
    }
}

/* returns the nuclear mass defect given the atomic mass, proton number and neutron number - this uses the calculated nuclear mass */
double Atom::calcMassDefectamuAlt(){
    if (Atom::useAccurate_){
        return ((this->protons_ * this->protonMass_ + this->neutrons_ * this->neutronMass_) - this->calcNuclearMass());
    } else {
        return ((this->protons_ * this->protonMassA_ + this->neutrons_ * this->neutronMassA_) - this->calcNuclearMass());
    }
}


/* returns the nuclear mass defect given the atomic mass, proton number and neutron number - this uses the hydrogen mass to give agreement with database values */
double Atom::calcMassDefectamu(){
    if (Atom::useAccurate_){
        return ((this->protons_ * this->hydrogenMass_ + this->neutrons_ * this->neutronMass_) - this->atomicMass_);
    } else {
        return ((this->protons_ * this->protonMassA_ + this->neutrons_ * this->neutronMassA_) - this->calcNuclearMass());
    }
}

/* returns the nuclear mass defect in kg */
double Atom::calcMassDefectkg(){
    if (Atom::useAccurate_){
        return (this->calcMassDefectamu() * this->amu_);
    } else {
        return (this->calcMassDefectamu() * this->amuA_);
    }
}

/* returns the total nuclear binding energy in J given the mass defect in kg */
double Atom::calcBindingEnergyJ(){
    if (Atom::useAccurate_){
        return (this->calcMassDefectkg() * std::pow(this->speedofLight_, 2));
    } else {
        return (this->calcMassDefectkg() * std::pow(this->speedofLightA_, 2));
    }
}

/* returns the total nuclear binding energy given the mass defect */
double Atom::calcBindingEnergykeV(){
    if (Atom::useAccurate_){
        return (this->calcMassDefectamu() * this->amutokeV_());
    } else {
        return (this->calcMassDefectamu() * this->amutokeVA_());
    }
}

/* returns the nuclear binding energy pernucleon given the total binding energy and number of nucleons */
double Atom::calcBindingEnergyperNucleonkeV(){
    return (this->calcBindingEnergykeV() / this->nucleons_);
}

/* getters for numerical constants */
double Atom::getElectronCharge(){
    if (Atom::useAccurate_) return this->electronCharge_;
    else return this->electronChargeA_;
}
double Atom::getAtomicMassUnit(){
    if (Atom::useAccurate_) return this->amu_;
    else return this->amuA_;
}
double Atom::getSpeedofLight(){
    if (Atom::useAccurate_) return this->speedofLight_;
    else return this->speedofLightA_;
}
double Atom::getElectronMass(){
    if (Atom::useAccurate_) return this->electronMass_;
    else return this->electronMassA_;
}
double Atom::getProtonMass(){
    if (Atom::useAccurate_) return this->protonMass_;
    else return this->protonMassA_;
}
double Atom::getNeutronMass(){
    if (Atom::useAccurate_) return this->neutronMass_;
    else return this->neutronMassA_;
}
