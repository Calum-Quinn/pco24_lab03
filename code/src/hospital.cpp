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
    interface->consoleAppendText(uniqueId, "Hospital ID: " + QString::number(getUniqueId()));
    
    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}

int Hospital::request(ItemType what, int qty){
    // TODO

    mutex.lock();

    // Verify the amount of patients available to send
    int delivered = qty <= stocks[what] ? qty : stocks[what];

    interface->consoleAppendText(uniqueId, "Sold: " + getItemName(what) + " " + QString::number(delivered) + " piece");

    // != 0 if available patients to send
    if(delivered){
        // Update stocks and availability of beds
        stocks[what] -= delivered;
        currentBeds -= delivered;
        money += delivered * TRANSFER_COST;
    }
    mutex.unlock();

    return delivered;
}

void Hospital::freeHealedPatient() {
    // TODO 

    // Free the patients that have stayed the mandatory 5 days after being healed
    stocks[ItemType::PatientHealed] -= healedStays.back();
    currentBeds -= healedStays.back();
    nbFree += healedStays.back();

    //interface->consoleAppendText(uniqueId, "Free patient");

    // Update the time stayed after being healed
    healedStays.insert(healedStays.begin(), 0);
    healedStays.pop_back();
}

void Hospital::transferPatientsFromClinic() {
    // TODO

    // Make sure there is an available bed
    if (currentBeds != maxBeds) {
        // Choose a clinic
        Seller* clinic = chooseRandomSeller(clinics);

        int cost = TRANSFER_COST + getEmployeeSalary(EmployeeType::Nurse);
        
        // Request a healed patient if sufficient money
        if (money >= cost && clinic->request(ItemType::PatientHealed,1)) {
            // Update current state of patients

            stocks[ItemType::PatientHealed]++;
            currentBeds++;
            nbHospitalised++;
            money -= cost;
            healedStays[0]++;

            interface->consoleAppendText(uniqueId, "Receive patient from clinic");
        }
    }
}

int Hospital::send(ItemType it, int qty, int bill) {
    // TODO

    mutex.lock();

    // Receiving a patient from an ambulance
    int availableBeds = maxBeds - currentBeds;
    int salary = getEmployeeSalary(EmployeeType::Nurse);

    // Make sure you only receive a patient if you there is sufficient space and money to pay for transfer + salary
    int received = qty <= availableBeds ? qty : availableBeds;
    received = money >= (received * (TRANSFER_COST + salary)) ? received : money / (TRANSFER_COST + salary);

    if(received){
        // Update amount of patients
        nbHospitalised += received;
        stocks[it] += received;
        currentBeds += received;
        money -= received * (TRANSFER_COST + salary);
    }

    mutex.unlock();

    return received * TRANSFER_COST;
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching its routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");

    healedStays = std::vector<int>(5, 0);

    while (!PcoThread::thisThread()->stopRequested()) {

        mutex.lock();

        transferPatientsFromClinic();

        freeHealedPatient();

        mutex.unlock();

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
