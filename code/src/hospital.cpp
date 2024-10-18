#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

IWindowInterface* Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");
    
    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}

int Hospital::request(ItemType what, int qty){
    // TODO

    // Receiving request for patients (probably sick ones, because the clinics use this function)
    if (what == ItemType::PatientSick) {
        
    }
    else {
        // Whatever is needed if healed patients are requested
    }
}

void Hospital::freeHealedPatient() {
    // TODO 
}

void Hospital::transferPatientsFromClinic() {
    // TODO

    int received = 0;

    // Request as many patients as the hospital can handle
    for(auto& clinic : clinics) {
        received = clinic->request(ItemType::PatientHealed, maxBeds - currentBeds);
        currentBeds += received;

        // If all beds are full, stop requesting more patients
        if (currentBeds == maxBeds) {
            break;
        }
    }
}

int Hospital::send(ItemType it, int qty, int bill) {
    // TODO
    // Receiving a patient from an ambulance and transferring directly to an available clinic
    nbHospitalised += qty;

    for (auto& clinic : clinics) {
        if (clinic->send(it,qty,bill)) {
            break;
        }
    }

    return qty;
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching its routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");

    while (true /*TODO*/) {
        transferPatientsFromClinic();

        freeHealedPatient();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        interface->simulateWork(); // Temps d'attente
    }

    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
}

int Hospital::getAmountPaidToWorkers() {
    return nbHospitalised * getEmployeeSalary(EmployeeType::Nurse);
}

int Hospital::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + nbFree;
}

std::map<ItemType, int> Hospital::getItemsForSale()
{
    return stocks;
}

void Hospital::setClinics(std::vector<Seller*> clinics){
    this->clinics = clinics;

    for (Seller* clinic : clinics) {
        interface->setLink(uniqueId, clinic->getUniqueId());
    }
}

void Hospital::setInterface(IWindowInterface* windowInterface){
    interface = windowInterface;
}
