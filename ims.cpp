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

int IntervalPrichoduObjednavej =       5;
const double dCasPrijatiaOsobne =       1;
const double dCasPrijatiaOnline =       1;
double dCasPripravyJedla =              17;
const double dCasBaleniaOnlineObj =     2;
const double dCasDoruceniaOsobne =      3;
double dCasDoruceniaOnline =            15;
const int CasOnlineTimeoutu =           60;
const int CasOsobnihoTimeoutu =         30;
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

        case 'a':
            //experiment 3 - zmena frekvencie osobnej objednavky
            IntervalPrichoduObjednavej = atof(optarg);
            break;

        default:
            fprintf(stderr, "Nespravny parameter.\n");
            exit(EXIT_FAILURE);
    }
}



/**
 * Event - timeout pro prijeti objednavek
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
        //nastaveny timeout
        Event *timeout = new Timeout(CasOnlineTimeoutu, this);

        //objednavka sa prijima pracovnikom
        Wait(Exponential(dCasPrijatiaOnline));

        //kuchar zoberie objednavku na vybavenie
        Enter(Kuchari, 1);

        //zruseni timeout
        delete timeout;

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

    //objednavky bez dovozu maji prednost
    Priority_t Priority = 1;

    void Behavior()
    {
        //nastaveni timeout
        Event *timeout = new Timeout(CasOsobnihoTimeoutu, this);

        //objednavka sa prijima casnikom
        Wait(Exponential(dCasPrijatiaOsobne));

        //zruseni timeoutu
        delete timeout;

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
        Activate(Time + Exponential(IntervalPrichoduObjednavej));
    }
};



int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    RandomSeed(time(NULL));

    //inicializacia simulacie s modelovym casom
    Init(0, dCasExperimentu);

    //aktivace generatoru
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