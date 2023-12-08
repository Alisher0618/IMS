#include <string>
#include <iostream>
#include <simlib.h>
#include <random>
#include <time.h>
using namespace std;

#define inputTime 5760

int inputAmount;
double finalVolume;

double changePercent;
double changePrice;

double checkpaste;

double price;



Stat STAT_produce_time("Average bottling time");

Facility FirstPump("Change location of tomato paste");
Store Ingredients("Other Ingredients", 1);
Facility Mixer("Mixer");
Facility Linka("Prenosova Linka");

class FinalProcess : public Process{
    void Behavior(){    
        cout << "Time " << Time << endl;    
        Seize(Linka);
        while(ketchup != 0){
            ketchup -= 10;
            Wait(Exponential(3));
            STAT_produce_time(Time);
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
        Wait(Exponential(10));
        Release(FirstPump);

        Enter(Ingredients, 1);
        Wait(1);
        (new SecondProcess)->Activate();
        finalVolume = inputAmount * 1.5;


        Seize(Mixer);
        Wait(30);
        Release(Mixer);

        (new FinalProcess)->Activate();
    }
};

double randomFraction() {
    srand(static_cast<unsigned int>(time(nullptr)));

    int randomNum = rand() % 100;

    double result = static_cast<double>(randomNum) / 100.0;
    return result;
}

class FirstProcess : public Process{
    void Behavior(){
        checkpaste = randomFraction();
        if(checkpaste > 0.05){  
            Wait(15);
            (new ContinueProcess)->Activate();
        }else{
            Wait(15);
            cout << "Control failed, leave the system " << endl;
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

int main(int argc, char** argv){
    if(argc != 2){
        cout << "invalid amount of arguments" << endl;
        return 0;
    }
    inputAmount = atoi(argv[1]);
    if(inputAmount < 1000){
        cout << "not enough tomato paste" << endl;
        return 0;
    }  

    srand(static_cast<unsigned int>(time(nullptr)));
    int randomNumber = rand() % 21;

    double startPrice = 89.9;
    price = startPrice; 

    if (isPrime(randomNumber)) {
        price = adjustedNumber(startPrice);
    }
    
    Init(0, inputTime);                 //
    (new EnterSystem)->Activate();      // start of simulation
    Run();                              //


    STAT_produce_time.Output();


    //Stats
    cout << "+----------------------------------------------------------+" << endl;
    cout << "+            Statistics                                    +" << endl;
    cout << "+----------------------------------------------------------+" << endl;

    cout << "Purchased " << inputAmount << "kg of tomato paste" << endl;
    cout << "Start price for tomatos is " << startPrice << " CZK/1kg" << endl;
    if(price != startPrice){
        cout << "Price has been changed! ";
        if(changePercent < 0){
            cout << "It decreased by " << changePercent * -1 << "%" << endl;
        }else if(changePercent > 0){
            cout << "It increased by " << changePercent << "%" << endl;
        }
        cout << "Now the price is " << price << " CZK/1kg" <<  endl;
    }
    cout << "+----------------------------------------------------------+" << endl;

    return 0;
}