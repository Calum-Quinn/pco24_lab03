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

    // Verify the amount of patients available to send
    int delivered = qty <= stocks[what] ? qty : stocks[what];

    // Update stocks and availability of beds
    stocks[what] -= delivered;
    currentBeds -= delivered;

    return delivered;
}

void Hospital::freeHealedPatient() {
    // TODO 
}

void Hospital::transferPatientsFromClinic() {
    // TODO

    // Make sure there is an available bed
    if (currentBeds != maxBeds) {
        // Choose a clinic
        Seller* clinic = chooseRandomSeller(clinics);
        
        // Request a healed patient
        if (clinic->request(ItemType::PatientHealed,1)) {
            // Update current state of patients
            stocks[ItemType::PatientHealed]++;
            currentBeds++;
        }
    }
}

int Hospital::send(ItemType it, int qty, int bill) {
    // TODO
    // Receiving a patient from an ambulance
    int availableBeds = maxBeds - currentBeds;

    // Make sure you only receive a patient if you there is sufficient space
    int received = qty <= availableBeds ? qty : availableBeds;

    // Update amount of patients
    nbHospitalised += received;
    stocks[it] += received;
    currentBeds += received;

    return received * TRANSFER_COST;
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
