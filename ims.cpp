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
double dPocetRozvozarov =               5;

double dIntervalPrichoduObjednavok =    10;
const double dCasPrijatiaOsobne =       1;
const double dCasPrijatiaOnline =       1;
double dCasPripravyJedla =              17;
const double dCasBaleniaOnlineObj =     2;
const double dCasDoruceniaOsobne =      3;
double dCasDoruceniaOnline =            15;
const int iCasOnlineTimeoutu =          75;
const int iCasOsobnehoTimeoutu =        45;
const int iCasRozvozTimeoutu =          60;
long canceled =                         0;


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
    std::string getoptStr = "+:k:r:j:d:a:";

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
            dCasPripravyJedla = atof(optarg);
            break;
        
        case 'd':
            dCasDoruceniaOnline = atof(optarg);
            break;

        case 'f':
            //experiment 3 - zmena frekvencie objednavok
            dIntervalPrichoduObjednavok = atof(optarg);
            break;

        default:
            fprintf(stderr, "Nespravny parameter.\n");
            exit(EXIT_FAILURE);
    }
}


/**
 * Event - timeout pre prijatie objednavok
*/
class Timeout : public Event
{
    Process *ptr;

    public:
        Timeout(double t, Process *p) : ptr(p)
        {
            Activate(Time+t);
        }

        void Behavior()
        {
            ptr->Out();
            delete ptr;
            Cancel();
            canceled++;
        }
};


/**
 * proces - tranzakcia online objednavky
*/
class OnlineObjednavka : public Process
{
    public : void Behavior()
    {
        //nastavenie timeout pre kuchara
        Event *timeoutK = new Timeout(iCasOnlineTimeoutu, this);

        //objednavka sa prijima pracovnikom
        Wait(Exponential(dCasPrijatiaOnline));

        //kuchar zoberie objednavku na vybavenie
        Enter(Kuchari, 1);

        //zrusenie timeout
        delete timeoutK;

        //samotne jedlo sa pripravuje
        Wait(Exponential(dCasPripravyJedla));
        
        //kuchar sa dokoncenim jedla uvolni
        Leave(Kuchari, 1);

        //objednavka caka na baliacu linku
        Seize(BaliacaLinka);

        //nastavenie timeout pre rozvozara
        Event *timeoutR = new Timeout(iCasRozvozTimeoutu, this);

        //zaberie rozvozara, kt. ju zabali
        Enter(Rozvozari, 1);

        //zrusenie timeout
        delete timeoutR;

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

    //objednavky bez dovozu maju prednost
    Priority_t Priority = 1;

    void Behavior()
    {
        //nastavenie timeout pre kuchara
        Event *timeoutK = new Timeout(iCasOsobnehoTimeoutu, this);

        //objednavka sa prijima casnikom
        Wait(Exponential(dCasPrijatiaOsobne));

        //zrusenie timeoutu
        delete timeoutK;

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
 * generator pre vytvaranie objednavok
*/
class GeneratorObjednavok : public Event
{
    void Behavior()
    {
        //60% sance, ze objednavka je online
        if(Random() <= 0.6)
        {
            (new OsobnaObjednavka)->Activate();
        }
        //40% sance, ze objednavka je s osobnim vyzvednutim
        else
        {
            (new OnlineObjednavka)->Activate();
        }
        Activate(Time + Exponential(dIntervalPrichoduObjednavok));
    }
};


int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    RandomSeed(time(NULL));

    //inicializacia simulacie s modelovym casom
    Init(0, dCasExperimentu);

    //aktivacia generatoru
    (new GeneratorObjednavok)->Activate();
    
    //spustenie simulacie
    Run();

    //vypis vysledkov simulacie
    cout << "OUTPUT_DATA" << endl;
    cout << "zrušené objednávky: " << canceled << endl;
    Kuchari.Output();
    Rozvozari.Output();
    BaliacaLinka.Output();

    return 0;
}