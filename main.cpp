#include <iostream>
#include <simlib.h>
#include <random>
#include <time.h>
#include <string.h>
#include <stdlib.h>
using namespace std;

#define inputTime 5760

int inputAmount;
double finalVolume;
double changePercent;
double changePrice;
double checkpaste;
double price;

double timeToProduce = 0;

int isfailure = 0;
int prior;

Stat STAT_bottling_time("Average bottling time");

Facility FirstPump("Change location of tomato paste");
Store Ingredients("Other Ingredients", 1);
Facility Mixer("Mixer");
Facility Linka("Prenosova Linka");

double randomFraction() {
    srand(static_cast<unsigned int>(time(nullptr)));

    int randomNum = rand() % 100;

    double result = static_cast<double>(randomNum) / 100.0;
    return result;
}

class FinalProcess : public Process{
    void Behavior(){    
        Seize(Linka);
        while(ketchup != 0){
            ketchup -= 10;
            timeToProduce = Time + Uniform(2, 3);
            Wait(Uniform(2, 3));
            STAT_bottling_time(Uniform(2, 3));
            
        }
        Release(Linka);
    }

    int ketchup = finalVolume;
};

class SecondProcess : public Process{
    void Behavior(){
        Wait(1);
        Leave(Ingredients, 1);
    }
};

class LeaveProcess : public Process {
    void Behavior() {
    }
};


class ContinueProcess : public Process {
    void Behavior() {    
        
        Seize(FirstPump);
        troublePump = randomFraction();

        if(troublePump <= 0.09 * prior){
            Wait(40);
            isfailure += 2;
        }
        Wait(Exponential(10));
        Release(FirstPump);


        Enter(Ingredients, 1);
        Wait(1);
        (new SecondProcess)->Activate();
        finalVolume = inputAmount * 1.5;


        Seize(Mixer);
        troubleMix = randomFraction();

        if(troubleMix <= 0.06 * prior){
            Wait(25);
            isfailure += 3;
        }
        

        Wait(30);
        Release(Mixer);
        (new FinalProcess)->Activate();
    }

    double troublePump, troubleMix;
};



class FirstProcess : public Process{
    void Behavior(){
        checkpaste = randomFraction();
        cout << checkpaste << endl;
        if(checkpaste > 0.05){  
            Wait(15);
            (new ContinueProcess)->Activate();
        }else{
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
    cout << "The first one is input amount of tomato paste, the second is property" << endl;
    cout << "Input amount must in range from 1000 to 7000" << endl;
    cout << "Priority must in range from 1 to 5, where 1 means the lowest and 5 highest priorities" << endl;
    cout << "Example: ./ims -a 1000 -p 1" << endl;
}

int main(int argc, char** argv){
    if((argc == 2 && strcmp(argv[1], "-h") == 0) || argc != 5){
        printHelp();
        return 0;
    }

    inputAmount = atoi(argv[2]);
    prior = atoi(argv[4]);
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
        cout << "| Producing time will be increased                         |" << endl; 
        cout << "+----------------------------------------------------------+" << endl;
        cout << endl;
    }else if(isfailure == 3){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ The mixer has broken down                                +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Producing time will be increased                         |" << endl; 
        cout << "+----------------------------------------------------------+" << endl;
        cout << endl;
    }else if(isfailure == 5){
        cout << "+----------------------------------------------------------+" << endl;
        cout << "+ Mixer and pump have broken down                          +" << endl;
        cout << "+----------------------------------------------------------+" << endl;
        cout << "| Producing time will be increased                         |" << endl; 
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

    cout << "+----------------------------------------------------------+" << endl;
    cout << "+ Procurement cost data                                    +" << endl;
    cout << "+----------------------------------------------------------+" << endl;
    spendMoney = price * inputAmount;
    cout << "| Company spent " << spendMoney << " crowns on tomato paste" <<  endl;

    cout << "| Company produced " << producedBottles << " bottles of ketchup" <<  endl;
    
    earnedMoney = priceforketchup * producedBottles;
    cout << "| Price per bottle is " << priceforketchup << " crowns." << endl;
    cout << "| Company earned " << earnedMoney << " crowns" << endl;

    cout << "+----------------------------------------------------------+" << endl;

    return 0;
}