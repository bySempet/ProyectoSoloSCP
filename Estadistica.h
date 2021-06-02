#include<stdio.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

int Obtener_edad( float alfa, float beta,int semilla)
{
    int edad;
    float mu = 100;
    gsl_rng *r;

    gsl_rng_env_setup();
    r = gsl_rng_alloc (gsl_rng_default);
    gsl_rng_set(r, semilla);

    edad = roundf(mu * gsl_ran_beta(r, alfa, beta));
    gsl_rng_free(r);
    
    return edad;
}

double calcular_PMuerte(int edad)
{
    if(edad>=0 && edad<10) return 0;
    else if(edad>=10 && edad<20) return 0.005;
    else if(edad>=20 && edad<30) return 0.01;
    else if(edad>=30 && edad<40) return 0.02;
    else if(edad>=40 && edad<50) return 0.04;
    else if(edad>=50 && edad<60) return 0.08;
    else if(edad>=60 && edad<70) return 0.08;
    else if(edad>=70 && edad<80) return 0.18;
    else return 0.2;
    
}

int morir_o_no(double pmuerte)
{
    double prob = rand()% 100;
    if(prob< (pmuerte*2)) return 1;
    else return 0;
}

int contagiar_o_no(double pmuerte)
{
    double prob = rand()% 100;
    if(prob< (pmuerte*2)) return 1;
    else return 0;
}

int cambio_direc(double pcambio)
{
    double prob = rand()% 100;
    if(prob< (pcambio*2)) return 1;
    else return 0;
}