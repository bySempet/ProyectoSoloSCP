#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"
#include "Estadistica.h"
#define Sano 0
#define Sin_Sintomas 1
#define Infectado 2
#define Recuperado 3    //Estados en los que se encuentra una persona
#define Vacunado 4
#define Fallecido 5
#define P_Contagiarse 0.3

struct Poblacion
{
  int individuos;
  int media_edad;
  int radio[2];
  double p_contagio;// Probabilidad de ser contagiado por otra persona
  int incubacion;
  int recuperacion;
  float cambio_vel;
};

struct Persona
{
  int id; //identificador de la persona
  int valido;  //Existe la persona o no
  int edad;   //Edad comprendida entre 0 y 110
  int estado;   //Puede encontrarse en 5 estados
  double p_muerte; //Probabilidad de morir al estar infectado
  int incubacion;  //Tiempo que lleva hasta terminar la incubacion
  int recuperacion; //Tiempo que lleva hasta terminar la incubacion
  int posicion[2];  //Posicion de la persona en el tablero
  int velocidad[2]; //Velocidad a la que se desplaza
};

void Crear_Tipo(int *id, int *valido, int *edad,int *estado,double *pmuerte,int *incubacion, int *recuperacion, int *posicion, int *velocidad, MPI_Datatype *PersonaType)
{
  int tam[9] = {1,1,1,1,1,1,1,2,2};
  MPI_Aint dist[9],dir1,dir2;
  MPI_Datatype tipo[9] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  dist[0] = 0;
  MPI_Get_address (id, &dir1);
  MPI_Get_address (valido, &dir2);
  dist[1] = dir2 - dir1;
  MPI_Get_address (edad, &dir2);
  dist[2] = dir2 - dir1;
  MPI_Get_address (estado, &dir2);
  dist[3] = dir2 - dir1;
  MPI_Get_address (pmuerte, &dir2);
  dist[4] = dir2 - dir1;
  MPI_Get_address (incubacion, &dir2);
  dist[5] = dir2 - dir1;
  MPI_Get_address (recuperacion, &dir2);
  dist[6] = dir2 - dir1;
  MPI_Get_address (posicion, &dir2);
  dist[7] = dir2 - dir1;
  MPI_Get_address (velocidad, &dir2);
  dist[8] = dir2 - dir1;

  MPI_Type_create_struct (9, tam , dist , tipo , PersonaType);
  MPI_Type_commit (PersonaType);

}

void Crear_Tipo_Poblacion(int individuos, int media_edad, int radio[2],double p_contagio,int incubacion,int recuperacion, float cambio_vel, MPI_Datatype *PoblacionType)
{
  int tam[7] = {1,1,2,1,1,1,1};
  MPI_Aint dist[7],dir1,dir2;
  MPI_Datatype tipo[7] = {MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE, MPI_INT, MPI_INT, MPI_FLOAT};
  dist[0] = 0;
  MPI_Get_address (&individuos, &dir1);
  MPI_Get_address (&media_edad, &dir2);
  dist[1] = dir2 - dir1;
  MPI_Get_address (&radio, &dir2);
  dist[2] = dir2 - dir1;
  MPI_Get_address (&p_contagio, &dir2);
  dist[3] = dir2 - dir1;
  MPI_Get_address (&incubacion, &dir2);
  dist[4] = dir2 - dir1;
  MPI_Get_address (&recuperacion, &dir2);
  dist[5] = dir2 - dir1;
  MPI_Get_address (&cambio_vel, &dir2);
  dist[6] = dir2 - dir1;

  MPI_Type_create_struct (7, tam , dist , tipo , PoblacionType);
  MPI_Type_commit(PoblacionType);
}

void Iniciar_Poblacion(struct Persona *Tablero, struct Poblacion *Poblacion,int tam_Poblacion,int tam_tablero)
{
  int i,j;
  int media_edad = 0;
  Poblacion->individuos = tam_Poblacion;
  Poblacion->radio[0]= 3;
  Poblacion->radio[1]= 3;               //Inicializar la estructura con sus datos, correcto menos la media debido al fallo con la cantidad de individuos.
  Poblacion->incubacion = 20;
  Poblacion->recuperacion = 20;
  Poblacion->p_contagio = P_Contagiarse;
  for(i = 0; i<tam_tablero; i++)
  {
    for(j = 0; j<tam_tablero; j++)
    {
      media_edad += Tablero[i*tam_tablero+j].edad;
    }
  }

  Poblacion->media_edad = media_edad / Poblacion->individuos; 
}
//Funcion que se encarga de inicializar toda la matriz "vacia"
void Crear_Poblacion(struct Persona *Tablero, int tam_tablero)
{
  int i,j,k;
  k=0;
  for(i = 0; i<tam_tablero; i++)
  {
    for(j = 0; j<tam_tablero; j++)
    {
      Tablero[i*tam_tablero+j].id = -1;
      Tablero[i*tam_tablero+j].valido = 0;
      Tablero[i*tam_tablero+j].edad = -1;
      Tablero[i*tam_tablero+j].estado = -1;
      Tablero[i*tam_tablero+j].p_muerte = 0;
      Tablero[i*tam_tablero+j].incubacion = 0;
      Tablero[i*tam_tablero+j].recuperacion = 0;
      Tablero[i*tam_tablero+j].posicion[0] = 0;
      Tablero[i*tam_tablero+j].posicion[1] = 0;
      Tablero[i*tam_tablero+j].velocidad[0] = 0;
      Tablero[i*tam_tablero+j].velocidad[1] = 0;
    }
  }
}
//funcion que resetea una casilla
void Reset_Casilla(struct Persona *Tablero, int pos)
{
  
  Tablero[pos].id = -1;
  Tablero[pos].valido = 0;
  Tablero[pos].edad = -1;
  Tablero[pos].estado = -1;
  Tablero[pos].p_muerte = 0;
  Tablero[pos].incubacion = 0;
  Tablero[pos].recuperacion = 0;
  Tablero[pos].posicion[0] = 0;
  Tablero[pos].posicion[1] = 0;
  Tablero[pos].velocidad[0] = 0;
  Tablero[pos].velocidad[1] = 0;
   
}
//Funcion que se encarga de situar todos los individuos por primera vez
void Situar_Poblacion(struct Persona *Tablero,int tam_tablero, int *posIndividuos, int paciente0,int personas)
{
  int i,semilla;
  semilla=0;i=0;
  int tam_vector= personas;
  for(i = 0; i<tam_vector; i++)
  {
    Tablero[posIndividuos[i]].id = i;     
    Tablero[posIndividuos[i]].valido = 1;
    semilla = rand() % 100;
    Tablero[posIndividuos[i]].edad = Obtener_edad(4,5,semilla); 
    if (i == paciente0) 
    {
      Tablero[posIndividuos[i]].estado = Sin_Sintomas;
      Tablero[posIndividuos[i]].incubacion = 0;
      Tablero[posIndividuos[i]].recuperacion = 0;
    }
    else 
    {
     Tablero[posIndividuos[i]].estado = Sano;
    }
    Tablero[posIndividuos[i]].posicion[0] = posIndividuos[i]/tam_tablero;
    Tablero[posIndividuos[i]].posicion[1] = posIndividuos[i]%tam_tablero;
    Tablero[posIndividuos[i]].p_muerte = calcular_PMuerte(Tablero[posIndividuos[i]].edad);
    Tablero[posIndividuos[i]].velocidad[0] = -3 + rand()% 6;
    Tablero[posIndividuos[i]].velocidad[1] = -3 + rand()% 6;
  }
}
void Ordenar_VectorLocal(int *posIndividuosLocal,int posicion_actual,int tam_poblacion)
{
  int j=0;
  if(posicion_actual==tam_poblacion-1)
  {
    posIndividuosLocal[posicion_actual]==0;
  }
  else
  {
    int aux =posicion_actual+1;
    for(j=posicion_actual;j<tam_poblacion-1;j++)
    {
      posIndividuosLocal[j]=posIndividuosLocal[aux];
      aux++;
    }
  }
}
void mover_persona(struct Persona *particion_Tablero,  int *posIndividuosLocal, int tam_fila,int tam_poblacion, int dimension_local, int my_rank, int world_size,int *cantidad_enviar,int *cantidad_enviar_mas,struct Persona *personas_a_enviar,struct Persona *personas_a_enviar_mas, MPI_Datatype *PersonaType)
{
  int i,j,x,y,id,tempx, tempy, noposible,temporal, posicion_vector_abajo,posicion_vector_arriba;
  int arriba,abajo;
  int num_filas = dimension_local/tam_fila;
  i=0;j=0;x=0;y=0;id=0;tempy=0;tempx=0; temporal=0;noposible=0;arriba=0; posicion_vector_abajo=0;posicion_vector_arriba=0;abajo=0;
  if(my_rank==0)
  {
    while(i<tam_poblacion && posIndividuosLocal[i]==0)
    {
      if(particion_Tablero[posIndividuosLocal[i]].estado != Fallecido  )
      {
        x = particion_Tablero[posIndividuosLocal[i]].velocidad[0];
        y = particion_Tablero[posIndividuosLocal[i]].velocidad[1];
        particion_Tablero[posIndividuosLocal[i]].velocidad[0] = -3 + rand()% 6;
        particion_Tablero[posIndividuosLocal[i]].velocidad[1] = -3 + rand()% 6;
        id = particion_Tablero[posIndividuosLocal[i]].id;
        tempx=particion_Tablero[posIndividuosLocal[i]].posicion[0]+x;
        tempy=particion_Tablero[posIndividuosLocal[i]].posicion[1]+y;
        if(tempx > (num_filas-1)){ abajo = 1; tempx=num_filas;}
        else if (tempx <= 0)tempx=0;
        if(tempy >= (tam_fila))tempy=(tam_fila-1);
        else if (tempy < 0)tempy=0;
        if(abajo == 0)
        {
          for(j=0;j<tam_poblacion;j++)
          {
            if(particion_Tablero[posIndividuosLocal[j]].posicion[0] == tempx && particion_Tablero[posIndividuosLocal[j]].posicion[1] == tempy && particion_Tablero[posIndividuosLocal[j]].id != id)
            {
              noposible = 1;
              break;
            }
          }
          if(noposible == 0 && (tempx!=particion_Tablero[posIndividuosLocal[i]].posicion[0] && tempy!=particion_Tablero[posIndividuosLocal[i]].posicion[1]))
          {
            temporal = posIndividuosLocal[i];
            posIndividuosLocal[i] = tempx*tam_fila+tempy;
            memcpy(&particion_Tablero[posIndividuosLocal[i]],&particion_Tablero[temporal],sizeof(struct Persona));
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            Reset_Casilla(particion_Tablero,temporal);
          }
          i++;
        }
        else
        {
          particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
          particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
          memcpy(&personas_a_enviar[posicion_vector_abajo],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
          Reset_Casilla(particion_Tablero,posIndividuosLocal[i]);
          posicion_vector_abajo++;
          Ordenar_VectorLocal(posIndividuosLocal,i,tam_poblacion);
        }
        abajo=0;
        noposible=0;
      }
    }
  }
  else if(world_size-1==my_rank)
  {
    while(i<tam_poblacion && posIndividuosLocal[i]==0)
    {
      if(particion_Tablero[posIndividuosLocal[i]].estado != Fallecido )
      {
        x = particion_Tablero[posIndividuosLocal[i]].velocidad[0];
        y = particion_Tablero[posIndividuosLocal[i]].velocidad[1];
        particion_Tablero[posIndividuosLocal[i]].velocidad[0] = -3 + rand()% 6;
        particion_Tablero[posIndividuosLocal[i]].velocidad[1] = -3 + rand()% 6;
        id = particion_Tablero[posIndividuosLocal[i]].id;
        tempx=particion_Tablero[posIndividuosLocal[i]].posicion[0]+x;
        tempy=particion_Tablero[posIndividuosLocal[i]].posicion[1]+y;
        if(tempx >= (num_filas*world_size))tempx = ((num_filas*world_size)-1);
        else if (tempx < (num_filas*(world_size-1))){arriba=1;tempx=(num_filas*world_size-1)-1;}
        if(tempy >= tam_fila)tempy=(tam_fila-1);
        else if (tempy < 0)tempy=0;
        if(arriba == 0)
        {
          for(j=0;j<tam_poblacion;j++)
          {
            if(particion_Tablero[posIndividuosLocal[j]].posicion[0] == tempx && particion_Tablero[posIndividuosLocal[j]].posicion[1] == tempy && particion_Tablero[posIndividuosLocal[j]].id != id)
            {
              noposible = 1;
              break;
            }
          }
          if(noposible == 0 && (tempx!=particion_Tablero[posIndividuosLocal[i]].posicion[0] && tempy!=particion_Tablero[posIndividuosLocal[i]].posicion[1]))
          {
            temporal = posIndividuosLocal[i];
            posIndividuosLocal[i] = ((tempx*tam_fila)-(num_filas*tam_fila*world_size-1))+tempy;
            memcpy(&particion_Tablero[posIndividuosLocal[i]],&particion_Tablero[temporal],sizeof(struct Persona));
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            Reset_Casilla(particion_Tablero,temporal);
          }
          i++;
        }
        else
        {
          particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
          particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
          memcpy(&personas_a_enviar_mas[ posicion_vector_arriba],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
          Reset_Casilla(particion_Tablero,posIndividuosLocal[i]);
          posicion_vector_arriba++;
          Ordenar_VectorLocal(posIndividuosLocal,i,tam_poblacion);
        }
        arriba=0;
        noposible=0;
      }
    }
  }
  else    
  {
    while(i<tam_poblacion && posIndividuosLocal[i]==0)
    {
      if(particion_Tablero[posIndividuosLocal[i]].estado != Fallecido )
      {
        x = particion_Tablero[posIndividuosLocal[i]].velocidad[0];
        y = particion_Tablero[posIndividuosLocal[i]].velocidad[1];
        particion_Tablero[posIndividuosLocal[i]].velocidad[0] = -3 + rand()% 6;
        particion_Tablero[posIndividuosLocal[i]].velocidad[1] = -3 + rand()% 6;
        id = particion_Tablero[posIndividuosLocal[i]].id;
        tempx=particion_Tablero[posIndividuosLocal[i]].posicion[0]+x;
        tempy=particion_Tablero[posIndividuosLocal[i]].posicion[1]+y;
        if(tempx >= (num_filas*(my_rank+1))){tempx = num_filas*(my_rank+1);abajo=1;}
        else if (tempx < my_rank*num_filas){arriba=1;tempx = (num_filas*my_rank)-1;}
        if(tempy >= (tam_fila))tempy=(tam_fila-1);
        else if (tempy < 0)tempx = 0;
        if(arriba == 0 && arriba == 0)
        {
          for(j=0;j<tam_poblacion;j++)
          {
            if(particion_Tablero[posIndividuosLocal[j]].posicion[0] == tempx && particion_Tablero[posIndividuosLocal[j]].posicion[1] == tempy && particion_Tablero[posIndividuosLocal[j]].id != id)
            {
              noposible = 1;
              break;
            }
          }
          if(noposible == 0 && (tempx!=particion_Tablero[posIndividuosLocal[i]].posicion[0] && tempy!=particion_Tablero[posIndividuosLocal[i]].posicion[1]))
          {
            temporal = posIndividuosLocal[i];
            posIndividuosLocal[i] = ((tempx*tam_fila)-(num_filas*tam_fila*(my_rank)))+tempy;
            memcpy(&particion_Tablero[posIndividuosLocal[i]],&particion_Tablero[temporal],sizeof(struct Persona));
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            Reset_Casilla(particion_Tablero,temporal);
          }
        }
        else
        {
          if(arriba==1)
          {
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            memcpy(&personas_a_enviar_mas[ posicion_vector_arriba],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
            Reset_Casilla(particion_Tablero,posIndividuosLocal[i]);
            posicion_vector_arriba++;
            Ordenar_VectorLocal(posIndividuosLocal,i,tam_poblacion);
          }
          else if(abajo==1)
          {
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            memcpy(&personas_a_enviar[ posicion_vector_abajo],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
            Reset_Casilla(particion_Tablero,posIndividuosLocal[i]);
            posicion_vector_abajo++;
            Ordenar_VectorLocal(posIndividuosLocal,i,tam_poblacion);
          }
        }
        arriba=0;
        abajo=0;
        noposible=0;
      }
    }
  }
  *cantidad_enviar = posicion_vector_abajo;
  *cantidad_enviar_mas = posicion_vector_arriba;
}
void cambiar_estado_poblacion(int *cantidad_enviar,int *cantidad_enviar_mas,int dimension_local,int tam_fila,int world_size,int world_rank,int *posIndividuosLocal,struct Persona *particion_Tablero,struct Poblacion *Poblacion,int tam_vector,int * Vacunados_Muertos,struct Persona * personas_a_enviar,struct Persona * personas_a_enviar_mas)
{
  printf("entro primera linea");
    fflush(stdin);
  int i,j,x,y,id,personas_en_particion,num_filas, cantidad1,cantidad2;
  personas_en_particion=0;
  for(i=0;i<tam_vector;i++)
  {
    if(posIndividuosLocal[i]==0) break;
    else personas_en_particion++;
  }
  id=0;y=0;x=0;
  num_filas=dimension_local/tam_fila;
  cantidad1=0;
  cantidad2=0;
  for(i = 0; i<personas_en_particion;i++) 
  {
    x=posIndividuosLocal[i];
    if(world_rank==0)
    {
      if((x/tam_fila)==(num_filas-1))
      {
        memcpy(&personas_a_enviar[cantidad1],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
        cantidad1++;
      }
    }
    else if(world_rank==world_size-1)
    {
      if(x<tam_fila)
      {
        memcpy(&personas_a_enviar_mas[cantidad2],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
        cantidad2++;
      }
    }
    else
    {
      if(x>=(tam_fila*(num_filas-1)))
      {
        memcpy(&personas_a_enviar[cantidad1],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
        cantidad1++;
      }
      else if(x<tam_fila)
      {
        memcpy(&personas_a_enviar_mas[cantidad2],&particion_Tablero[posIndividuosLocal[i]],sizeof(struct Persona));
        cantidad2++;
      }
    }

    if((particion_Tablero[posIndividuosLocal[i]].estado == Infectado || particion_Tablero[posIndividuosLocal[i]].estado == Sin_Sintomas) )
    {printf("entro");
    fflush(stdin);
      if(morir_o_no(Poblacion->p_contagio))
      {

      	particion_Tablero[posIndividuosLocal[i]].estado = Fallecido;
      	particion_Tablero[posIndividuosLocal[i]].velocidad[0] = 0;
      	particion_Tablero[posIndividuosLocal[i]].velocidad[1] = 0;
        Vacunados_Muertos[1]++;
      }
      else if(particion_Tablero[posIndividuosLocal[i]].estado == Sin_Sintomas)
      {

       if(particion_Tablero[posIndividuosLocal[i]].incubacion < Poblacion->incubacion)particion_Tablero[posIndividuosLocal[i]].incubacion++;
       else 
       {
         particion_Tablero[posIndividuosLocal[i]].estado = Infectado;
         particion_Tablero[posIndividuosLocal[i]].incubacion= 0;
       }
      }
      else if(particion_Tablero[posIndividuosLocal[i]].estado == Infectado)
      {
        if(particion_Tablero[posIndividuosLocal[i]].recuperacion < Poblacion->recuperacion)
        particion_Tablero[posIndividuosLocal[i]].recuperacion++;
        else
        { 
          particion_Tablero[posIndividuosLocal[i]].estado = Recuperado;
          particion_Tablero[posIndividuosLocal[i]].recuperacion = 0;
          Vacunados_Muertos[3]++;
        }
      }

    }
    else if(particion_Tablero[posIndividuosLocal[i]].estado == Sano)
    {
printf("entro sano");
    fflush(stdin);
     x = particion_Tablero[posIndividuosLocal[i]].posicion[0];
     y = particion_Tablero[posIndividuosLocal[i]].posicion[1];
     id = particion_Tablero[posIndividuosLocal[i]].id;
     for(j=0;j<personas_en_particion;j++)
     {
       if(particion_Tablero[posIndividuosLocal[j]].id == id);
       else if(((particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]+3) ||(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]-3))&& (particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]))
       {

         if(particion_Tablero[posIndividuosLocal[j]].estado== Infectado || particion_Tablero[posIndividuosLocal[j]].estado == Sin_Sintomas)
          {
           if(contagiar_o_no(particion_Tablero[posIndividuosLocal[j]].p_muerte))
           {
             particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
             Vacunados_Muertos[2]++;
             break;
           }
          }
       }
       else if(((particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]+2) ||(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]-2))&&(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]))
       { 
         if(particion_Tablero[posIndividuosLocal[j]].estado== Infectado || particion_Tablero[posIndividuosLocal[j]].estado == Sin_Sintomas)
          {
           if(contagiar_o_no(particion_Tablero[posIndividuosLocal[j]].p_muerte))
           {
             particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
             Vacunados_Muertos[2]++;
             break;
           }
          }
       }
       else if(((particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]+1) ||(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]-1))&&(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]))
       { 
         if(particion_Tablero[posIndividuosLocal[j]].estado== Infectado || particion_Tablero[posIndividuosLocal[j]].estado == Sin_Sintomas)
          {
           if(contagiar_o_no(particion_Tablero[posIndividuosLocal[j]].p_muerte))
           {
             particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
             Vacunados_Muertos[2]++;
             break;
           }
          }
       }
       else if(((particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]+3) ||(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]-3))&&(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]))
       { 
        if(particion_Tablero[posIndividuosLocal[j]].estado== Infectado || particion_Tablero[posIndividuosLocal[j]].estado == Sin_Sintomas)
        {
	        if(contagiar_o_no(particion_Tablero[posIndividuosLocal[i]].p_muerte))
          {
            particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
            Vacunados_Muertos[2]++;
            break;
          }
        }
       }
       else if(((particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]+2) ||(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]-2))&&(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]))
       { 
        if(particion_Tablero[posIndividuosLocal[j]].estado== Infectado || particion_Tablero[posIndividuosLocal[j]].estado == Sin_Sintomas)
        {
	        if(contagiar_o_no(particion_Tablero[posIndividuosLocal[i]].p_muerte))
          {
            particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
            Vacunados_Muertos[2]++;
            break;
          }
        }
       }
       else if(((particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]+1) ||(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]-1))&&(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]))
       {
        if(particion_Tablero[posIndividuosLocal[j]].estado== Infectado || particion_Tablero[posIndividuosLocal[j]].estado == Sin_Sintomas)
        {
	        if(contagiar_o_no(particion_Tablero[posIndividuosLocal[i]].p_muerte))
          {
            particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
            Vacunados_Muertos[2]++;
            break;
          }
        }
       }
     }
    }
  }

  *cantidad_enviar = cantidad1;
  *cantidad_enviar_mas = cantidad2;
}
//Funcion para cambio de estado mirando los procesadores vecinos
//La idea es que el procesador que ejecute envie a sus compañeros colindantes la posicion de una persona del borde
//Y estos le respondan si existe alguien o no y luego el calcule la probabilidad de contagiarse si esa persona esta contagiada
void cambiar_estado_poblacion_bordes(int recepcion,int recepcion_mas, struct Persona *personas_a_enviar, struct Persona *personas_a_enviar_mas ,int dimension_particion,int tam_Poblacion,int *posIndividuosLocal,struct Persona *particion_Tablero,int * Vacunados_Muertos, int tam_fila, int my_rank,int world_size, MPI_Datatype PersonaType)
{
  int i,j,personas_en_particion;
  int numero_filas = dimension_particion/tam_fila;
  personas_en_particion=0;
  for(i=0;i<tam_Poblacion;i++)
  {
    if(posIndividuosLocal[i]==0) break;
    else personas_en_particion++;
  }
  if(my_rank==0)
  {
    for(i = 0; i<recepcion;i++)
    {
     for(j=0; j<personas_en_particion;j++)
     {
       if((particion_Tablero[posIndividuosLocal[j]].posicion[0]== personas_a_enviar[i].posicion[0]-1)&&(particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar[i].posicion[1]-1 || particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar[i].posicion[1] ||particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar[i].posicion[1]+1))
       {
         if((personas_a_enviar[i].estado== Infectado ||personas_a_enviar[i].estado== Sin_Sintomas) && particion_Tablero[posIndividuosLocal[j]].estado== Sano)
         {
           if(contagiar_o_no(personas_a_enviar[i].p_muerte))
          {
            particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
            Vacunados_Muertos[2]++;
            break;
          }
         }
       }
     }
    }
  }
  else if(my_rank == world_size-1)
  {
    for(i = 0; i<recepcion_mas;i++)
    {
     for(j=0; j<tam_Poblacion;j++)
     {
       if((particion_Tablero[posIndividuosLocal[j]].posicion[0]== personas_a_enviar_mas[i].posicion[0]+1)&&(particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar_mas[i].posicion[1]-1 || particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar_mas[i].posicion[1] ||particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar_mas[i].posicion[1]+1))
       {
         if((personas_a_enviar_mas[i].estado== Infectado ||personas_a_enviar_mas[i].estado== Sin_Sintomas) && particion_Tablero[posIndividuosLocal[j]].estado== Sano)
         {
           if(contagiar_o_no(personas_a_enviar_mas[i].p_muerte))
          {
            particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
            Vacunados_Muertos[2]++;
            break;
          }
         }
       }
     }
    }
  }
  else
  {
    for(i = 0; i<recepcion;i++)
    {
     for(j=0; j<tam_Poblacion;j++)
     {
       if((particion_Tablero[posIndividuosLocal[j]].posicion[0]== personas_a_enviar[i].posicion[0]-1)&&(particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar[i].posicion[1]-1 || particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar[i].posicion[1] ||particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar[i].posicion[1]+1))
       {
         if((personas_a_enviar[i].estado== Infectado ||personas_a_enviar[i].estado== Sin_Sintomas) && particion_Tablero[posIndividuosLocal[j]].estado== Sano)
         {
           if(contagiar_o_no(personas_a_enviar[i].p_muerte))
          {
            particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
            Vacunados_Muertos[2]++;
            break;
          }
         }
       }
     }
    }
    for(i = 0; i<recepcion_mas;i++)
    {
     for(j=0; j<tam_Poblacion;j++)
     {
       if((particion_Tablero[posIndividuosLocal[j]].posicion[0]== personas_a_enviar_mas[i].posicion[0]+1)&&(particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar_mas[i].posicion[1]-1 || particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar_mas[i].posicion[1] ||particion_Tablero[posIndividuosLocal[j]].posicion[1]== personas_a_enviar_mas[i].posicion[1]+1))
       {
         if((personas_a_enviar_mas[i].estado== Infectado ||personas_a_enviar_mas[i].estado== Sin_Sintomas) && particion_Tablero[posIndividuosLocal[j]].estado== Sano)
         {
           if(contagiar_o_no(personas_a_enviar_mas[i].p_muerte))
          {
            particion_Tablero[posIndividuosLocal[i]].estado = Sin_Sintomas;
            Vacunados_Muertos[2]++;
            break;
          }
         }
       }
     }
    }
  }
}
void vacunar_persona(struct Persona *particion_Tablero, int *posIndividuosLocal,int vacunaDiaria,int tam_pobla, int * Vacunados_Muertos)
{
  int j,vacunas;
  j=0;vacunas= 0;
    if (vacunaDiaria>=1)
    {
     while(vacunas < vacunaDiaria && posIndividuosLocal[j] != 0)
     {
	    if(particion_Tablero[posIndividuosLocal[j]].estado != Fallecido && particion_Tablero[posIndividuosLocal[j]].estado != Vacunado)
   	  {
	     particion_Tablero[posIndividuosLocal[j]].estado = Vacunado;
       Vacunados_Muertos[0]++;
       vacunas++;
   	  }
      else j++;
    }
   }
}
//posIndividuosLocal contendra las personas dentro de la matriz del procesador que invoque la funcion
void Personas_en_mi(struct Persona *particion_Tablero,int* posIndividuosLocal,int dimension_local,int tam_poblacion)
{
  int i,j;
  j= 0;
  for(i=0;i<dimension_local;i++)
  {
    if(particion_Tablero[i].valido=1)
    {
      posIndividuosLocal[j]=i;
      j++;
    }
  }
  for(i=j;i<tam_poblacion;i++) posIndividuosLocal[i]=0;
 
}
void colocar_personas(struct Persona *particion_Tablero,int* posIndividuosLocal,int recepcion,int recepcion_mas,struct Persona* personas_a_enviar,struct Persona* personas_a_enviar_mas,int world_size,int my_rank,int tam_fila, int dimension_local)
{
  int x,y,posicion_colocar,id,i,j,z;
  id=0;posicion_colocar=0;x=0;y=0;z=0;
  int num_filas= dimension_local/tam_fila;
  int iteraciones= recepcion;
  int iteraciones_mas= recepcion_mas;
  if(my_rank==0)
  {
    for(i=0; i<iteraciones;i++)
    {
      x=personas_a_enviar[i].posicion[0];
      y=personas_a_enviar[i].posicion[1];
      posicion_colocar = (x-(num_filas*tam_fila*my_rank))+y;
      id=personas_a_enviar[i].id;
      if(particion_Tablero[posicion_colocar].valido==0)
      {
       memcpy(&particion_Tablero[posicion_colocar],&personas_a_enviar[i],sizeof(struct Persona));
       while(posIndividuosLocal[j]!=0)j++;
       posIndividuosLocal[j]=posicion_colocar;
       j=0;
      }
      else
      {
       while(particion_Tablero[j].valido!=1)j++;
       memcpy(&particion_Tablero[j],&personas_a_enviar[i],sizeof(struct Persona));
       while(posIndividuosLocal[z]!=0)z++;
       posIndividuosLocal[z]=j;
       j=0;z=0;
      }
    
    }
  }
  else if(my_rank==world_size-1)
  {
    for(i=0; i<iteraciones_mas;i++)
    {
      x=personas_a_enviar_mas[i].posicion[0];
      y=personas_a_enviar_mas[i].posicion[1];
      posicion_colocar = (x-(num_filas*tam_fila*my_rank))+y;
      id=personas_a_enviar_mas[i].id;
      if(particion_Tablero[posicion_colocar].valido==0)
      {
       memcpy(&particion_Tablero[posicion_colocar],&personas_a_enviar_mas[i],sizeof(struct Persona));
       while(posIndividuosLocal[j]!=0)j++;
       posIndividuosLocal[j]=posicion_colocar;
       j=0;
      }
      else
      {
       while(particion_Tablero[j].valido!=1)j++;
       memcpy(&particion_Tablero[j],&personas_a_enviar_mas[i],sizeof(struct Persona));
       while(posIndividuosLocal[z]!=0)z++;
       posIndividuosLocal[z]=j;
       j=0;z=0;
      }
    
    }
  }
  else
  {
    for(i=0; i<iteraciones_mas;i++)
    {
      x=personas_a_enviar_mas[i].posicion[0];
      y=personas_a_enviar_mas[i].posicion[1];
      posicion_colocar = (x-(num_filas*tam_fila*my_rank))+y;
      id=personas_a_enviar_mas[i].id;
      if(particion_Tablero[posicion_colocar].valido==0)
      {
       memcpy(&particion_Tablero[posicion_colocar],&personas_a_enviar_mas[i],sizeof(struct Persona));
       while(posIndividuosLocal[j]!=0)j++;
       posIndividuosLocal[j]=posicion_colocar;
       j=0;
      }
      else
      {
       while(particion_Tablero[j].valido!=1)j++;
       memcpy(&particion_Tablero[j],&personas_a_enviar_mas[i],sizeof(struct Persona));
       while(posIndividuosLocal[z]!=0)z++;
       posIndividuosLocal[z]=j;
       j=0;z=0;
      }
    }
    for(i=0; i<iteraciones;i++)
    {
      x=personas_a_enviar[i].posicion[0];
      y=personas_a_enviar[i].posicion[1];
      posicion_colocar = (x-(num_filas*tam_fila*my_rank))+y;
      id=personas_a_enviar[i].id;
      if(particion_Tablero[posicion_colocar].valido==0)
      {
       memcpy(&particion_Tablero[posicion_colocar],&personas_a_enviar[i],sizeof(struct Persona));
       while(posIndividuosLocal[j]!=0)j++;
       posIndividuosLocal[j]=posicion_colocar;
       j=0;
      }
      else
      {
       while(particion_Tablero[j].valido!=1)j++;
       memcpy(&particion_Tablero[j],&personas_a_enviar[i],sizeof(struct Persona));
       while(posIndividuosLocal[z]!=0)z++;
       posIndividuosLocal[z]=j;
       j=0;z=0;
      }
    
    }
  }
  
}
void inicializar_envios(struct Persona * personas_a_enviar,struct Persona *personas_a_enviar_mas,int tam_Poblacion)
{
  int j=0;
  for(j = 0; j<tam_Poblacion; j++)
    {
      personas_a_enviar_mas[j].id = -1;
      personas_a_enviar_mas[j].valido = 0;
      personas_a_enviar_mas[j].edad = -1;
      personas_a_enviar_mas[j].estado = -1;
      personas_a_enviar_mas[j].p_muerte = 0;
      personas_a_enviar_mas[j].incubacion = 0;
      personas_a_enviar_mas[j].recuperacion = 0;
      personas_a_enviar_mas[j].posicion[0] = 0;
      personas_a_enviar_mas[j].posicion[1] = 0;
      personas_a_enviar_mas[j].velocidad[0] = 0;
      personas_a_enviar_mas[j].velocidad[1] = 0;

      personas_a_enviar[j].id = -1;
      personas_a_enviar[j].valido = 0;
      personas_a_enviar[j].edad = -1;
      personas_a_enviar[j].estado = -1;
      personas_a_enviar[j].p_muerte = 0;
      personas_a_enviar[j].incubacion = 0;
      personas_a_enviar[j].recuperacion = 0;
      personas_a_enviar[j].posicion[0] = 0;
      personas_a_enviar[j].posicion[1] = 0;
      personas_a_enviar[j].velocidad[0] = 0;
      personas_a_enviar[j].velocidad[1] = 0;
    }
}

int main(int argc, char* argv[]) 
{
  
  if(argc<5)
  {
      printf("Parametos necesarios: Tamaño de Matriz, N de Poblacion,Tiempo de Simulacion y Porcentaje de vacunados\n");
      MPI_Finalize();
      return 2;
  }
 
  int world_rank,world_size;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&world_size);
  MPI_Comm_rank(MPI_COMM_WORLD,&world_rank); 
  if(atoi(argv[1])%world_size!= 0)
  {
    printf("Dimension de la matriz no divisible por el numero de procesadores\n");
    MPI_Finalize();
    return 2;
  }
  double t0,t1;
  int particion,tam_tablero,tam_Poblacion,Simulacion,pVacunacion;
  float vacunaDiaria;
  srand((time)NULL);
  char *v= NULL;
  

  tam_tablero= strtol(argv[1],&v,10);
  tam_Poblacion = strtol(argv[2],&v,10);
  Simulacion = strtol(argv[3],&v,10);
  pVacunacion = strtol(argv[4],&v,10);
  vacunaDiaria=(float) ((float)(tam_Poblacion*(pVacunacion/100.0))/Simulacion);
  particion = tam_tablero/world_size;

  if(tam_Poblacion>((tam_tablero*tam_tablero)/2))
  {
    printf("La poblacion debe de ser menor o igual la mitad del tablero \n");
    MPI_Finalize();
    return 2;
  }
 struct Persona *Tablero = malloc(sizeof(struct Persona)*(tam_tablero*tam_tablero));
  
  int tamano_poblacion = sizeof(struct Poblacion);
  printf("tamano_poblacion:%d\n",tamano_poblacion);
  fflush(stdin);
  struct Poblacion *Poblacion = malloc(tamano_poblacion);
  int *posIndividuos =(int*)malloc(sizeof(int)*tam_Poblacion);
  int i = 0;
  int x,paciente0;
  int igual=0;
 if(world_rank== 0)
 {
   while(i<tam_Poblacion)
   {
     x =rand()%(tam_tablero*tam_tablero);
     int j;
     for(j=0; j<i; j++)
     {
       if(x==posIndividuos[j])
       {
         igual=1;
         break;
       }
     }
     if(igual == 0)
     {
       posIndividuos[i] = x;
       i++;
     }
     igual=0;
    }
   paciente0 = rand()% tam_Poblacion; //seleccion del paciente 0 que será el primer infectado
   printf("Hola, soy el paciente 0 y mi id es %d\n",paciente0);

   Crear_Poblacion(Tablero,tam_tablero);  //Iniciar la matriz vacia de personas
   Situar_Poblacion(Tablero,tam_tablero,posIndividuos,paciente0,tam_Poblacion);  //Iniciamos la matriz con personas
   Iniciar_Poblacion(Tablero,Poblacion,tam_Poblacion,tam_tablero);   //Rellenamos los datos de la poblacion en la estructura
   t0 = MPI_Wtime();
 }

  int dimension_local = tam_tablero*particion;
  struct Persona *particion_Tablero = malloc(dimension_local*sizeof(struct Persona));
  MPI_Datatype PersonaType;
  Crear_Tipo(&(Tablero->id),&(Tablero->valido),&(Tablero->edad),&(Tablero->estado),&(Tablero->p_muerte),&(Tablero->incubacion),&(Tablero->recuperacion),Tablero->posicion,Tablero->velocidad,&PersonaType);
 
  if(world_rank==0)
  {
    for(i=0;i<tam_Poblacion;i++)
    {
      printf("Soy la persona %d y estoy en esta posicion %d %d y mi velocidad %d %d \n",Tablero[posIndividuos[i]].id,Tablero[posIndividuos[i]].posicion[0],Tablero[posIndividuos[i]].posicion[1],Tablero[posIndividuos[i]].velocidad[0],Tablero[posIndividuos[i]].velocidad[1]);
    }
  }
  MPI_Scatter(Tablero,dimension_local,PersonaType,particion_Tablero,dimension_local,PersonaType,0,MPI_COMM_WORLD);

  int *posIndividuosLocal = (int*)malloc(tam_Poblacion*sizeof(int));
  Personas_en_mi(particion_Tablero,posIndividuosLocal,dimension_local,tam_Poblacion);
  free(posIndividuos);

  int t=0;
  int vacunas=0;
  int *Vacunados_Muertos = (int *)malloc(5* sizeof(int));
  for(i=0;i<5;i++) Vacunados_Muertos[i]=0;
  float tempVacuna=0.0;
  struct Persona *personas_a_enviar = malloc(tam_Poblacion*sizeof(struct Persona));
  struct Persona *personas_a_enviar_mas = malloc(tam_Poblacion*sizeof(struct Persona));
  inicializar_envios(personas_a_enviar,personas_a_enviar_mas,tam_Poblacion);
  int cantidad_enviar = 0;
  int cantidad_enviar_mas = 0;
  int recepcion=0;
  int recepcion_mas = 0;
  int enviado_abajo= world_rank+1;
  int enviado_arriba = world_rank-1;
  if(enviado_abajo==world_size)enviado_abajo= world_size-1;
  if(enviado_arriba<0)enviado_arriba= 0;

  //Bucle principal del comportamiento de las personas
  while( t < Simulacion ) 
  {
    printf("Hola, soy el procesador %d y empiezo estados local iteracion %d \n",world_rank,t);
    fflush(stdin);
    cambiar_estado_poblacion(&cantidad_enviar,&cantidad_enviar_mas,dimension_local,tam_tablero,world_size,world_rank,posIndividuosLocal,particion_Tablero,Poblacion,tam_Poblacion, Vacunados_Muertos,personas_a_enviar,personas_a_enviar_mas);
    //Comunicaciones para los tamaños de los vectores que contienen las personas en los bordes estado
    MPI_Send(&cantidad_enviar, 1, MPI_INT,enviado_abajo, 0, MPI_COMM_WORLD);
    MPI_Send(&cantidad_enviar_mas,1, MPI_INT,enviado_arriba, 0, MPI_COMM_WORLD);
    MPI_Recv(&recepcion, 1, MPI_INT,enviado_abajo,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&recepcion_mas,1, MPI_INT,enviado_arriba,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //Comunicaciones de envio y recepcion de las personas para estado en los bordes estado
    MPI_Send(personas_a_enviar, cantidad_enviar, PersonaType,enviado_abajo, 0, MPI_COMM_WORLD);
    MPI_Send(personas_a_enviar_mas, cantidad_enviar_mas, PersonaType,enviado_arriba, 0, MPI_COMM_WORLD);
    MPI_Recv(personas_a_enviar, recepcion, PersonaType,enviado_abajo,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(personas_a_enviar_mas, recepcion_mas, PersonaType,enviado_arriba,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Hola, soy el procesador %d y empezar estados bordes iteracion %d,valor de recepcion:%d y recepcion_mas:%d \n",world_rank,t,recepcion,recepcion_mas);
    fflush(stdin);
    cambiar_estado_poblacion_bordes(recepcion,recepcion_mas,personas_a_enviar,personas_a_enviar_mas,dimension_local,tam_Poblacion,posIndividuosLocal,particion_Tablero,Vacunados_Muertos, tam_tablero, world_rank,world_size, PersonaType);
    printf("Hola, soy el procesador %d y estados terminados ahora a mover iteracion %d \n",world_rank,t);
    fflush(stdin);
    mover_persona(particion_Tablero, posIndividuosLocal,tam_tablero, tam_Poblacion, dimension_local, world_rank,world_size,&cantidad_enviar,&cantidad_enviar_mas,personas_a_enviar,personas_a_enviar_mas, &PersonaType);
    printf("Hola, soy el procesador %d y mover terminado iteracion %d \n",world_rank,t);
    fflush(stdin);
    //Comunicaciones para los tamaños de los vectores que contienen las personas a mover
    MPI_Send(&cantidad_enviar, 1, MPI_INT,enviado_abajo, 0, MPI_COMM_WORLD);
    MPI_Send(&cantidad_enviar_mas,1, MPI_INT,enviado_arriba, 0, MPI_COMM_WORLD);
    MPI_Recv(&recepcion, 1, MPI_INT,enviado_abajo,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&recepcion_mas,1, MPI_INT,enviado_arriba,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //Comunicaciones de envio y recepcion de las personas a mover
    MPI_Send(personas_a_enviar, cantidad_enviar, PersonaType,enviado_abajo, 0, MPI_COMM_WORLD);
    MPI_Recv(personas_a_enviar,recepcion , PersonaType,enviado_abajo,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    MPI_Send(personas_a_enviar_mas, cantidad_enviar_mas, PersonaType,enviado_arriba, 0, MPI_COMM_WORLD);
    MPI_Recv(personas_a_enviar_mas, recepcion_mas, PersonaType,enviado_arriba,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Hola, soy el procesador %d y empezar colocar iteracion %d \n",world_rank,t);
    fflush(stdin);
    colocar_personas(particion_Tablero,posIndividuosLocal,recepcion,recepcion_mas,personas_a_enviar,personas_a_enviar_mas,world_size,world_rank,tam_tablero,dimension_local);
    if(world_rank==0)
    {
     tempVacuna = vacunaDiaria+tempVacuna;
     while(tempVacuna >=1)
     {
      vacunas++;                        //El proceso de vacunacion debe realizarlo un unico procesador
      tempVacuna--;                     //Para que no todos vacunen el mismo numero de personas y se inmunice a la poblacion instantaneamente
     }
     vacunar_persona(particion_Tablero, posIndividuosLocal,vacunas,tam_Poblacion,Vacunados_Muertos);
     vacunas=0;
     printf("Hola, soy el procesador %d y vacunacion %d terminada \n",world_rank,t);
     fflush(stdin);
    }
   
    printf("Hola, soy el procesador %d y termino la iteracion %d \n",world_rank,t);
    fflush(stdin);
    t++;

  }
  
  MPI_Gather(Vacunados_Muertos, 5, MPI_INT, Vacunados_Muertos, 5, MPI_INT, 0, MPI_COMM_WORLD);//Recogida de datos de los procesadores
  MPI_Gather(particion_Tablero, dimension_local, PersonaType, Tablero, dimension_local, PersonaType, 0, MPI_COMM_WORLD);//Unificar la matriz para impresion final de datos

  if(world_rank==0)
  {
   t1 = MPI_Wtime();
   printf("\nTej + Comunicaciones : %1.3f ms\n", (t1 - t0)*1000);

   FILE *fb;
 	 fb = fopen( "valores.pos", "w");
   int z;
   for(z=0;z<tam_Poblacion*tam_Poblacion;z++)
   {
     if(Tablero[z].id!=-1)
     fprintf(fb,"Soy yo %d y estoy en la posicion %d %d y estoy %d y mivelocidad es de %d %d\n",Tablero[z].id,Tablero[z].posicion[0],Tablero[z].posicion[1],Tablero[z].estado,Tablero[z].velocidad[0],Tablero[z].velocidad[1]);
   }
   fclose(fb);
   FILE *fp;
   fp = fopen ( "valores.metrica", "w" );   
   fprintf(fp,"Numero de vacunados: %d, Numero de Fallecidos: %d, Numero de Contagiados: %d, Numero de Recuperados: %d \n",Vacunados_Muertos[0],Vacunados_Muertos[1],Vacunados_Muertos[2],Vacunados_Muertos[3]);
   fclose ( fp );
  }
  
  free(particion_Tablero);
  free(posIndividuosLocal);
  free(Vacunados_Muertos);
  free(Poblacion);
  free(Tablero);
  free(personas_a_enviar);
  free(personas_a_enviar_mas);
  MPI_Finalize();
}