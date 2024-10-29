#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>

IWindowInterface* Clinic::interface = nullptr;

Clinic::Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId), nbTreated(0), resourcesNeeded(resourcesNeeded)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Factory created");

    for(const auto& item : resourcesNeeded) {
        stocks[item] = 0;
    }
}

bool Clinic::verifyResources() {
    for (auto item : resourcesNeeded) {
        if (stocks[item] == 0) {
            return false;
        }
    }
    return true;
}

int Clinic::request(ItemType what, int qty){
    // TODO

    mutex.lock();

    // Check to see if there are patients to transfer back to the hospital after treatment
    int transferred = stocks[what] >= qty ? qty : stocks[what];

    // Update stocks depending on transfer (0 if none to transfer)
    stocks[what] -= transferred;
    money += transferred * TRANSFER_COST;

    mutex.unlock();

    return transferred;
}

void Clinic::treatPatient() {
    // TODO 

    // If you reach this point it means you have the necessary items to treat a patient
    ItemType item1 = resourcesNeeded[1];
    ItemType item2 = resourcesNeeded[2];

    //Temps simulant un traitement 
    interface->simulateWork();

    // TODO
    // Update all the stocks (items and patients)
    stocks[item1]--;
    stocks[item2]--;
    stocks[ItemType::PatientSick]--;
    stocks[ItemType::PatientHealed]++;
    nbTreated++;
    money += HEALING_COST;
    
    interface->consoleAppendText(uniqueId, "Clinic have healed a new patient");
}

void Clinic::orderResources() {
    // TODO

    Seller* hospital = chooseRandomSeller(hospitals);
    if (money >= getCostPerUnit(ItemType::PatientSick) && hospital->request(ItemType::PatientSick, 1)) {
        stocks[ItemType::PatientSick]++;
    }

    // No need to check if we have the necessary resources because we would not be here if it was the case
    for(auto& supplier : suppliers) {
        for(auto& item : resourcesNeeded) {
            // Check if the clinic has enough money to buy the supplies
            if (money >= getCostPerUnit(item)) {
                int cost = supplier->request(item, 1);

                // If the transaction goes through, update the state of the clinic
                if (cost) {
                    stocks[item]++;
                    money -= cost;
                }
            }
        }
    }
}

void Clinic::run() {
    if (hospitals.empty() || suppliers.empty()) {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }
    interface->consoleAppendText(uniqueId, "[START] Factory routine");

    while (true /*TODO*/) {
        
        if (verifyResources()) {
            treatPatient();
        } else {
            orderResources();
        }
       
        interface->simulateWork();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Factory routine");
}


void Clinic::setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;

    for (Seller* hospital : hospitals) {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
    for (Seller* supplier : suppliers) {
        interface->setLink(uniqueId, supplier->getUniqueId());
    }
}

int Clinic::getTreatmentCost() {
    return 0;
}

int Clinic::getWaitingPatients() {
    return stocks[ItemType::PatientSick];
}

int Clinic::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed];
}

int Clinic::send(ItemType it, int qty, int bill) {
    stocks[it] += qty;
}

int Clinic::getAmountPaidToWorkers() {
    return nbTreated * getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
}

void Clinic::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::map<ItemType, int> Clinic::getItemsForSale() {
    return stocks;
}


Pulmonology::Pulmonology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Thermometer}) {}

Cardiology::Cardiology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Syringe, ItemType::Stethoscope}) {}

Neurology::Neurology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Scalpel}) {}
