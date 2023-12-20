#include <iostream>
#include <simlib.h>
#include <random>
#include <time.h>
#include <string.h>
#include <stdlib.h>
using namespace std;

#define inputTime 5760
#define COST_PUMP 5000
#define COST_MIXER 3500


int inputAmount;
double finalVolume;
double changePercent;
double changePrice;
double checkpaste;
double price;

double timeToProduce = 0;
double timeToRepair = 0;
double repairCostPump = 0;
double repairCostMixer = 0;


int isfailure = 0;
int prior;
int troublefree = 1;

Stat STAT_bottling_time("Average bottling and wrapping up time");

Facility Pump("Change location of tomato paste");
Store Ingredients("Other Ingredients", 1);
Facility Mixer("Mixer");
Facility Linka("Prenosova Linka");

//function returns random number from 0 to 0.9999
double randomFraction() {
    srand(static_cast<unsigned int>(time(nullptr)));

    int randomNum = rand() % 100;

    double result = static_cast<double>(randomNum) / 100.0;
    return result;
}

//bottling and wrapping up process
class FinalProcess : public Process{
    void Behavior(){    
        Seize(Linka);
        while(ketchup != 0){
            ketchup -= 10;
            if(prior <= 3){
                timeToProduce = Time + Uniform(2, 3);
                Wait(Uniform(2, 3));
                STAT_bottling_time(Uniform(2, 3));
            }else{
                timeToProduce = Time + Uniform(1, 2);
                Wait(Uniform(1, 2));
                STAT_bottling_time(Uniform(1, 2));
            }

            
        }
        Release(Linka);
    }

    int ketchup = finalVolume;
};

class AddIngredients : public Process{
    void Behavior(){
        Wait(1);
        Leave(Ingredients, 1);
    }
};

//leave the system in case of failing thickness checking
class LeaveProcess : public Process {
    void Behavior() {
    }
};


class ContinueProcess : public Process {
    void Behavior() {    
        
        Seize(Pump);
        troublePump = randomFraction();

        //probability of pump failure
        if(troublePump <= 0.09 * (1 + prior/10) * (1 + inputAmount / 10000) && troublefree == 1){
            Wait(80 * prior + inputAmount / 10);
            timeToRepair += 80 * prior + inputAmount / 10;
            repairCostPump += COST_PUMP * prior * (inputAmount / 1000);
            isfailure += 2;
        }

        Wait(Exponential(11) - prior);
        Release(Pump);

        //adding other ingredients
        Enter(Ingredients, 1);
        Wait(1);
        (new AddIngredients)->Activate();
        finalVolume = inputAmount * 1.5;


        Seize(Mixer);
        troubleMix = randomFraction();

        //probability of mixer failure
        if(troubleMix <= 0.06 * (1 + prior/10) * (1 + finalVolume / 10000) && troublefree == 1){
            Wait(40 * prior  + finalVolume / 10);
            timeToRepair += 40 * prior  + finalVolume / 10;
            repairCostMixer += COST_MIXER * prior * (finalVolume / 1000);
            isfailure += 3;
        }
        
        Wait(32 - prior * 2);
        Release(Mixer);
        (new FinalProcess)->Activate();
    }

    double troublePump, troubleMix;
};



class FirstProcess : public Process{ //checking thickness of tomato paste
    void Behavior(){
        checkpaste = randomFraction();
        if(checkpaste > 0.05 || troublefree == 0){  //checking passed
            Wait(15);
            (new ContinueProcess)->Activate();
        }else{                                      //checking failed
            Wait(15);
            isfailure = 1;
            (new LeaveProcess)->Activate();
        }
    }
};

class EnterSystem : public Event{
    void Behavior(){
        (new FirstProcess)->Activate();
        Activate(Time + Exponential(inputTime)); //input = 5780h = 4 days
    }

};

// function for getting random number for changing price
// this was used to model price changes
double adjustedNumber(double originalNumber) {
    std::srand(static_cast<unsigned int>(time(nullptr)));

    double percentage = (rand() % 21 - 10) / 100.0;
    double deviation = originalNumber * percentage;
    changePercent = percentage * 100;
    changePrice = deviation;

    return originalNumber + deviation;
}

bool isPrime(int num) {
    if (num <= 1) {
        return false;
    }
    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}

void printHelp(){
    cout << "To start program properly you need to write 2 arguments" << endl;
    cout << "The first one is input amount of tomato paste, the second is property, the third is optinal and means trouble-free running" << endl;
    cout << "Input amount must in range from 1000 to 7000" << endl;
    cout << "Priority must in range from 1 to 5, where 1 means the lowest and 5 highest priorities" << endl;
    cout << "Trouble-free running mode can be enabled by value 0, otherwise the value is always 1" << endl;
    cout << "Example: ./ims -a 1000 -p 1 -m 0" << endl;
}

int main(int argc, char** argv){
    if((argc == 2 && strcmp(argv[1], "-h") == 0) || (argc != 5 && argc != 7)){
        printHelp();
        return 0;
    }

    inputAmount = atoi(argv[2]);
    prior = atoi(argv[4]);
    if(argc == 7){
        troublefree = atoi(argv[6]);
    }
    if(inputAmount < 1000){
        cout << "Not enough tomato paste" << endl;
        cout << "Input ./ims -h to show help" << endl;
        return 0;
    }else if(inputAmount > 7000){
        cout << "To much tomato paste" << endl;
        cout << "Input ./ims -h to show help" << endl;
        return 0;
    }  

    if(!(prior >= 1 && prior <= 5)){
        cout << "Wrong input prioroity" << endl;
        cout << "Input ./ims -h to show help" << endl;
        return 0;
    }

    if(troublefree != 0 && troublefree != 1){
        cout << "Wrong input trouble-free mode" << endl;
        cout << "Input ./ims -h to show help" << endl;
        return 0;
    }

    srand(static_cast<unsigned int>(time(nullptr)));
    int randomNumber = rand() % 21;

    double producedBottles = (inputAmount * 1.5) / 0.5;
    double spendMoney, earnedMoney;
    double priceforketchup = 69.9;

    double startPrice = 25;
    price = startPrice; 

    if (isPrime(randomNumber)) {
        price = adjustedNumber(startPrice);
    }
    
    Init(0, inputTime);                 //
    (new EnterSystem)->Activate();      // start of simulation
    Run();                              //

    if(isfailure == 2){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ The pump has broken down                                 +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Producing time will be increased by " << timeToRepair << endl; 
        cout << "| Spent on pump repair " << repairCostPump << " crowns" << endl; 
        cout << "+----------------------------------------------------------+" << endl;
        cout << endl;
    }else if(isfailure == 3){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ The mixer has broken down                                +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Producing time will be increased by " << timeToRepair << endl; 
        cout << "| Spent on mixer repair " << repairCostMixer << " crowns" << endl;; 
        cout << "+----------------------------------------------------------+" << endl;
        cout << endl;
    }else if(isfailure == 5){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ Mixer and pump have broken down                          +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Producing time will be increased by " << timeToRepair << endl;
        cout << "| Spent on repair " << repairCostPump + repairCostMixer << " crowns" << endl; 
        cout << "+----------------------------------------------------------+" << endl;
        cout << endl;
    }else if(isfailure == 0){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ Producing went without troubles                          +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Producing time will not be increased                     |" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << endl;
    }


    if(isfailure != 1){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ STATISTIC Average producing time                         +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Average value = " << timeToProduce << endl; 
        cout << "+----------------------------------------------------------+" << endl;
        cout << endl;

        STAT_bottling_time.Output();

        cout << endl;
    }else if(isfailure == 1){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ Produce failed                                           +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Reason : Thickness test failed! Stop production          |" << endl; 
        cout << "+----------------------------------------------------------+" << endl;

        cout << endl;
    }
    
    

    
    cout << "+----------------------------------------------------------+" << endl;
    cout << "+ Input data                                               +" << endl;
    cout << "+----------------------------------------------------------+" << endl;

    cout << "| Purchased " << inputAmount << "kg of tomato paste" << endl;
    cout << "| Start price for tomato paste is " << startPrice << " CZK/1kg" << endl;
    if(price != startPrice){
        cout << "| Price has been changed! ";
        if(changePercent < 0){
            cout << "It decreased by " << changePercent * -1 << "%" << endl;
        }else if(changePercent > 0){
            priceforketchup = priceforketchup * (1 + changePercent / 100);
            cout << "It increased by " << changePercent << "%" << endl;
        }
        cout << "| Now the price is " << price << " CZK/1kg" <<  endl;
    }
    cout << "+----------------------------------------------------------+" << endl;
    
    cout << endl;

    if(isfailure == 1){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ Procurement cost data                                    +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        spendMoney = price * inputAmount;
        cout << "| Company spent " << spendMoney << " crowns on tomato paste" <<  endl;

        cout << "| Company could produced " << producedBottles << " bottles of ketchup" <<  endl;
        
        earnedMoney = priceforketchup * producedBottles;
        cout << "| Price per bottle is " << priceforketchup << " crowns" << endl;
        cout << "| Company could earned " << earnedMoney << " crowns" << endl;
        cout << "| Net profit would be " << earnedMoney - spendMoney << " crowns" << endl;

        cout << "+----------------------------------------------------------+" << endl;
    }else{
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ Procurement cost data                                    +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        spendMoney = price * inputAmount;
        cout << "| Company spent " << spendMoney << " crowns on tomato paste" <<  endl;

        cout << "| Company produced " << producedBottles << " bottles of ketchup" <<  endl;
        
        earnedMoney = priceforketchup * producedBottles;
        cout << "| Price per bottle is " << priceforketchup << " crowns." << endl;
        cout << "| Company earned " << earnedMoney << " crowns" << endl;
        if(repairCostPump || repairCostMixer){
            int repairCostAll = repairCostMixer + repairCostPump;
            cout << "| Net profit including repair is " << earnedMoney - spendMoney - repairCostAll<< " crowns" << endl;
        }else{
            cout << "| Net profit is " << earnedMoney - spendMoney << " crowns" << endl;
        }
        

        cout << "+----------------------------------------------------------+" << endl;
    }

    

    return 0;
}