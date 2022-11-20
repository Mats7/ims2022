#include <simlib.h>
#include <iostream>
#include <getopt.h>
using namespace std;

const double v_dobaExperimentu =    365*24*60;

void parseArguments(int argc, char *argv[])
{
    int c;
    std::string getoptStr = "+:a:b:c:";

    while((c = getopt (argc, argv, getoptStr.c_str())) != -1)
    switch(c)
    {	
        case 'a':

            break;
		
        case 'b':

            break;

        case 'c':

            break;

        default:
            fprintf(stderr, "Nespravny parameter.\n");
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    parseArguments(argc, argv);

    Init(0, v_casExperimentu);
    
    Run();

    return 0;
}