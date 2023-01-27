#include <iostream>
#include "Simulation.h"


Simulation SimInstance = Simulation();
Simulation* pSimInstance = &SimInstance;

int main()
{
    pSimInstance->render(1000, 1000, 0.5);
}