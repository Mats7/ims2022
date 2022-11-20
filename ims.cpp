#include <simlib.h>
#include <iostream>
#include <getopt.h>
using namespace std;


const double v_casExperimentu =         365*24*60;
double v_pocetKucharov =                5;
double v_pocetRozvozarov =              3;

const double v_casPrichoduOnlineObj =   7;
const double v_casPrichoduOsobnychObj = 10;
const double v_casPrijatiaOsobne =      1;
const double v_casPrijatiaOnline =      1;
const double v_casSpracovaniaOsobne =   13;
const double v_casSpracovaniaOnline =   13;
const double v_casBaleniaOnlineObj =    2;
const double v_casDoruceniaOsobne =     3;
const double v_casDoruceniaOnline =     15;


//Kuchari pripravujuci jedlo
Store Kuchari("Kuchari", v_pocetKucharov);

//Rozvozari online objednavok
Store Rozvozari("Rozvozari", v_pocetRozvozarov);

//Balenie online objednavok 
Facility BaliacaLinka("Linka na pripravu online objednavok");


void parseArguments(int argc, char *argv[])
{
    int c;
    std::string getoptStr = "+:a:b:c:";

    while((c = getopt (argc, argv, getoptStr.c_str())) != -1)
    switch(c)
    {	
        case 'a':
            //experiment 1
            break;
		
        case 'b':
            //experiment 2
            break;

        case 'c':
            //experiment 3
            break;

        default:
            fprintf(stderr, "Nespravny parameter.\n");
            exit(EXIT_FAILURE);
    }
}


class OnlineObjednavka : public Process
{
    void Behavior()
    {
        //objednavka sa prijima pracovnikom
        Wait(Exponential(v_casPrijatiaOnline));

        //kuchar zoberie objednavku na vybavenie
        Enter(Kuchari, 1);
        
        //samotne jedlo sa pripravuje
        Wait(Exponential(v_casSpracovaniaOnline));
        
        //kuchar sa dokoncenim jedla uvolni
        Leave(Kuchari, 1);

        //objednavka caka na baliacu linku
        Seize(BaliacaLinka);

        //zaberie rozvozara
        Enter(Rozvozari, 1);

        //objednavka sa bali na odvoz
        Wait(Exponential(v_casBaleniaOnlineObj));

        //uvolni sa baliaca zona
        Release(BaliacaLinka);

        //objednavka sa dorucuje
        Wait(Exponential(v_casBaleniaOnlineObj));

        //uvolni sa rozvozar
        Leave(Rozvozari, 1);  
    }
};

class OsobnaObjednavka : public Process
{
    void Behavior()
    {
        //objednavka sa prijima casnikom
        Wait(Exponential(v_casPrijatiaOsobne));

        //kuchar zoberie objednavku na vybavenie
        Enter(Kuchari, 1);
        
        //samotne jedlo sa pripravuje
        Wait(Exponential(v_casSpracovaniaOnline));
        
        //kuchar sa dokoncenim jedla uvolni
        Leave(Kuchari, 1);

        //objednavka sa odnesie
        Wait(Exponential(v_casDoruceniaOsobne)); 
    }
};


class GeneratorOnlineObjednavok : public Event
{
    void Behavior()
    {
        (new OnlineObjednavka)->Activate();
        Activate(Time + Exponential(v_casPrichoduOnlineObj));
    } 
};

class GeneratorOsobnychObjednavok : public Event
{
    void Behavior()
    {
        (new OsobnaObjednavka)->Activate();
        Activate(Time + Exponential(v_casPrichoduOsobnychObj));
    } 
};


int main(int argc, char *argv[])
{
    parseArguments(argc, argv);

    Init(0, v_casExperimentu);
    
    Run();

    return 0;
}