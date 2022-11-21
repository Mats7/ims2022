/*
*   IMS - Modelování a simulace: 
*   Projekt - SHO v logistice
*   Autori: Matej Cimmerman <xcimme00>; Lukáš Kristek <xkrist24>
*/

#include <simlib.h>
#include <iostream>
#include <getopt.h>
using namespace std;

//casove jednotky su v minutach
const double dCasExperimentu =          365*24*60;
double dPocetKucharov =                 5;
double dPocetRozvozarov =               3;

const double dCasPrichoduOnlineObj =    7;
const double dCasPrichoduOsobnychObj =  10;
const double dCasPrijatiaOsobne =       1;
const double dCasPrijatiaOnline =       1;
double dCasPripravyJedla =              17;
const double dCasBaleniaOnlineObj =     2;
const double dCasDoruceniaOsobne =      3;
double dCasDoruceniaOnline =            15;


//Kuchari pripravujuci jedlo
Store Kuchari("Kuchari", dPocetKucharov);

//Rozvozari online objednavok
Store Rozvozari("Rozvozari", dPocetRozvozarov);

//Balenie online objednavok 
Facility BaliacaLinka("Linka na pripravu online objednavok");


/**
 * ziskavanie argumentov pre vykonanie experimentov
*/
void parseArguments(int argc, char *argv[])
{
    int c;
    std::string getoptStr = "+:k:r:j:d:";

    while((c = getopt (argc, argv, getoptStr.c_str())) != -1)
    switch(c)
    {	
        case 'k':
            //experiment 1 - zmena poctu kucharov
            dPocetKucharov = atof(optarg);
            Kuchari.SetCapacity(dPocetKucharov);
            break;
		
        case 'r':
            //experiment 2 - zmena poctu rozvozarov
            dPocetRozvozarov = atof(optarg);
            Rozvozari.SetCapacity(dPocetRozvozarov);
            break;

        case 'j':
            //experiment 3 - zmena trvania pripravy jedla
            dCasPripravyJedla = atof(optarg);
            break;
        
        case 'd':
            //experiment 4 - zmena trvania dopravy online objednavky
            dCasDoruceniaOnline = atof(optarg);
            break;

        default:
            fprintf(stderr, "Nespravny parameter.\n");
            exit(EXIT_FAILURE);
    }
}


/**
 * proces - tranzakcia online objednavky
*/
class OnlineObjednavka : public Process
{
    void Behavior()
    {
        //objednavka sa prijima pracovnikom
        Wait(Exponential(dCasPrijatiaOnline));

        //kuchar zoberie objednavku na vybavenie
        Enter(Kuchari, 1);
        
        //samotne jedlo sa pripravuje
        Wait(Exponential(dCasPripravyJedla));
        
        //kuchar sa dokoncenim jedla uvolni
        Leave(Kuchari, 1);

        //objednavka caka na baliacu linku
        Seize(BaliacaLinka);

        //zaberie rozvozara, kt. ju zabali
        Enter(Rozvozari, 1);

        //objednavka sa bali na odvoz
        Wait(Exponential(dCasBaleniaOnlineObj));

        //uvolni sa baliaca linka
        Release(BaliacaLinka);

        //objednavka sa dorucuje
        Wait(Exponential(dCasBaleniaOnlineObj));

        //uvolni sa rozvozar
        Leave(Rozvozari, 1);  
    }
};

/**
 * proces - tranzakcia osobnej objednavky
*/
class OsobnaObjednavka : public Process
{
    void Behavior()
    {
        //objednavka sa prijima casnikom
        Wait(Exponential(dCasPrijatiaOsobne));

        //kuchar zoberie objednavku na vybavenie
        Enter(Kuchari, 1);
        
        //samotne jedlo sa pripravuje
        Wait(Exponential(dCasPripravyJedla));
        
        //kuchar sa dokoncenim jedla uvolni
        Leave(Kuchari, 1);

        //objednavka sa odnesie
        Wait(Exponential(dCasDoruceniaOsobne)); 
    }
};


/**
 * generator pre vytvaranie online objednavok
*/
class GeneratorOnlineObjednavok : public Event
{
    void Behavior()
    {
        (new OnlineObjednavka)->Activate();
        Activate(Time + Exponential(dCasPrichoduOnlineObj));
    } 
};

/**
 * generator pre vytvaranie osobnych objednavok
*/
class GeneratorOsobnychObjednavok : public Event
{
    void Behavior()
    {
        (new OsobnaObjednavka)->Activate();
        Activate(Time + Exponential(dCasPrichoduOsobnychObj));
    } 
};


int main(int argc, char *argv[])
{
    parseArguments(argc, argv);

    //aktivacia generatorov procesov
    (new OsobnaObjednavka)->Activate();
    (new OnlineObjednavka)->Activate();

    //inicializacia simulacie s modelovym casom
    Init(0, dCasExperimentu);
    
    //spustenie simulacie
    Run();

    //vypis vysledkov simulacie
    cout << "OUTPUT_DATA" << endl;
    Kuchari.Output();
    Rozvozari.Output();
    BaliacaLinka.Output();

    return 0;
}