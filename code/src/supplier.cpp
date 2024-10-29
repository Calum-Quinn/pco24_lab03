#include "supplier.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

IWindowInterface* Supplier::interface = nullptr;

Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbSupplied(0) 
{
    for (const auto& item : resourcesSupplied) {    
        stocks[item] = 0;    
    }

    interface->consoleAppendText(uniqueId, QString("Supplier Created"));
    interface->consoleAppendText(uniqueId, "Supplier ID: " + QString::number(getUniqueId()));

    for (const auto& item : resourcesSupplied) {
        interface->consoleAppendText(uniqueId, QString("Supplier will sell: " + getItemName(item)));
    }

    interface->updateFund(uniqueId, fund);
}


int Supplier::request(ItemType it, int qty) {
    // TODO

    mutex.lock();

    // Check availability of desired products
    int delivered = qty <= stocks[it] ? qty : stocks[it];

    interface->consoleAppendText(uniqueId, "Sold: " + getItemName(it) + " " + QString::number(delivered) + " piece");

    if(delivered){
        // Update stock of products
        stocks[it] -= delivered;
        nbSupplied += delivered;
        money += delivered * getCostPerUnit(it);
    }

    mutex.unlock();

    // Return cost of delivered products
    return delivered * getCostPerUnit(it);
}

void Supplier::run() {
    interface->consoleAppendText(uniqueId, "[START] Supplier routine");


    while (!PcoThread::thisThread()->stopRequested()) {
        ItemType resourceSupplied = getRandomItemFromStock();

        mutex.lock();

        int supplierCost = getEmployeeSalary(getEmployeeThatProduces(resourceSupplied));
        // TODO

        // Obtain a new random item
        if ((std::find(this->resourcesSupplied.begin(), this->resourcesSupplied.end(), resourceSupplied) != this->resourcesSupplied.end())
                && money >= supplierCost) {

            stocks[resourceSupplied]++;
            money -= supplierCost;

            nbSupplied++;

            interface->consoleAppendText(uniqueId, "Resupplied: " + getItemName(resourceSupplied) + " " + QString::number(1) + " piece");
        }

        mutex.unlock();

        /* Temps aléatoire borné qui simule l'attente du travail fini*/
        interface->simulateWork();
        //TODO

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Supplier routine");
}


std::map<ItemType, int> Supplier::getItemsForSale() {
    return stocks;
}

int Supplier::getMaterialCost() {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

int Supplier::getAmountPaidToWorkers() {
    return nbSupplied * getEmployeeSalary(EmployeeType::Supplier);
}

void Supplier::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::vector<ItemType> Supplier::getResourcesSupplied() const
{
    return resourcesSupplied;
}

int Supplier::send(ItemType it, int qty, int bill){
    return 0;
}
