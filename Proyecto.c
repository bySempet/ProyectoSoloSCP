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

void Crear_Tipo(int *id, int *valido, int *edad,int *estado,double *pmuerte,int *incubacion, int *recuperacion, int *posicion[2], int *velocidad[2], MPI_Datatype *PersonaType)
{
  int tam[9] = {1,1,1,1,1,1,1,2,2};
  MPI_Aint dist[9],dir1,dir2;
  MPI_Datatype tipo[9] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  dist[0] = 0;
  MPI_Get_address (id, &dir1);
  MPI_Get_address (&valido, &dir2);
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
  MPI_Get_address (&posicion, &dir2);
  dist[7] = dir2 - dir1;
  MPI_Get_address (&velocidad, &dir2);
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
      Tablero[i*tam_tablero+j].edad = 0;
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
  Tablero[pos].estado = 0;
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
  int i,j,k,semilla;
  int tam_vector= personas;
  for(i = 0; i<tam_vector; i++)
  {
    Tablero[posIndividuos[i]].id = i;     //Se queda bloqueado aqui desconozco por que
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
void mover_persona(struct Persona *particion_Tablero, int *posIndividuos, int *posIndividuosLocal, int tam_fila,int tam_poblacion, int dimension_Local, int my_rank, int world_size, MPI_Datatype *PersonaType)
{
  int i,j,k,x,y,id,tempx, tempy, noposible,temporal,auxX,auxY;
  int posicion,arriba,abajo;
  struct Persona *temp = malloc(sizeof(struct Persona)*tam_fila);
  struct Persona *temp2 = malloc(sizeof(struct Persona)*tam_fila);
  int num_filas = dimension_Local/tam_fila;
  noposible = 0;
  abajo = 0;arriba=0;
  if(my_rank==0)
  {
    for(i = 0; i<tam_poblacion;i++)
    {
      if(particion_Tablero[posIndividuosLocal[i]].estado != Fallecido && particion_Tablero[posIndividuosLocal[i]].valido == 1 )
      {
        x = particion_Tablero[posIndividuosLocal[i]].velocidad[0];
        y = particion_Tablero[posIndividuosLocal[i]].velocidad[1];
        particion_Tablero[posIndividuosLocal[i]].velocidad[0] = -3 + rand()% 6;
        particion_Tablero[posIndividuosLocal[i]].velocidad[1] = -3 + rand()% 6;
        id = particion_Tablero[posIndividuosLocal[i]].id;
        tempx=particion_Tablero[posIndividuosLocal[i]].posicion[0]+x;
        tempy=particion_Tablero[posIndividuosLocal[i]].posicion[1]+y;
        if(tempx >= (num_filas-1)){abajo = 1; tempx=num_filas;}
        else if (tempx < 0)tempx=0;
        if(tempy >= (num_filas-1))tempy=(num_filas-1);
        else if (tempy < 0)tempy=0;
        if(abajo == 0)
        {
          for(j=0;j<tam_poblacion;j++)
          {
            if(particion_Tablero[posIndividuosLocal[j]].id == id);
            else if(particion_Tablero[posIndividuosLocal[j]].posicion[0] == tempx && particion_Tablero[posIndividuosLocal[j]].posicion[1] == tempy)
            {
              noposible = 1;
              break;
            }
          }
          if(noposible == 0 && (tempx!=particion_Tablero[posIndividuosLocal[i]].posicion[0] && tempy!=particion_Tablero[posIndividuosLocal[i]].posicion[0]))
          {
            temporal = posIndividuosLocal[i];
            posIndividuosLocal[i] = tempx*tam_poblacion+tempy;
            memcpy(&particion_Tablero[posIndividuosLocal[i]],&particion_Tablero[temporal],sizeof(struct Persona));
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            Reset_Casilla(particion_Tablero,temporal);
          }
        }
        else
        {
          MPI_Send(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
          MPI_Recv(&temp[0],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
          for(k=0;k<tam_fila;k++)
          {
            if(temp[k].valido == 0 && temp[k].posicion[1] == tempy)
            {
              temporal = posIndividuosLocal[i];
              posIndividuosLocal[i] = tempx*tam_poblacion+tempy;
              memcpy(&temp[k],&particion_Tablero[temporal],sizeof(struct Persona));
              temp[k].posicion[0]=tempx;
              temp[k].posicion[1]=tempy;
              Reset_Casilla(particion_Tablero,temporal);
	      break;
            }
          }
          MPI_Send(&temp[0],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
          MPI_Recv(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
        abajo=0;
        noposible=0;
      }
    }
  }
  else if(world_size-1==my_rank)
  {
    for(i = 0; i<tam_poblacion;i++)
    {
      if(particion_Tablero[posIndividuosLocal[i]].estado != Fallecido && particion_Tablero[posIndividuosLocal[i]].valido == 1 )
      {
        x = particion_Tablero[posIndividuosLocal[i]].velocidad[0];
        y = particion_Tablero[posIndividuosLocal[i]].velocidad[1];
        particion_Tablero[posIndividuosLocal[i]].velocidad[0] = -3 + rand()% 6;
        particion_Tablero[posIndividuosLocal[i]].velocidad[1] = -3 + rand()% 6;
        id = particion_Tablero[posIndividuosLocal[i]].id;
        tempx=particion_Tablero[posIndividuosLocal[i]].posicion[0]+x;
        tempy=particion_Tablero[posIndividuosLocal[i]].posicion[1]+y;
        if(tempx >= (num_filas*world_size-1))tempx = (num_filas*world_size-1);
        else if (tempx < 0){arriba=1;tempx=-1;}
        if(tempy >= (num_filas-1))tempy=(num_filas-1);
        else if (tempy < 0)tempy=0;
        if(arriba == 0)
        {
          for(j=0;j<tam_poblacion;j++)
          {
            if(particion_Tablero[posIndividuosLocal[j]].id == id);
            else if(particion_Tablero[posIndividuosLocal[j]].posicion[0] == tempx && particion_Tablero[posIndividuosLocal[j]].posicion[1] == tempy)
            {
              noposible = 1;
              break;
            }
          }
          if(noposible == 0 && (tempx!=particion_Tablero[posIndividuosLocal[i]].posicion[0] && tempy!=particion_Tablero[posIndividuosLocal[i]].posicion[0]))
          {
            temporal = posIndividuosLocal[i];
            posIndividuosLocal[i] = tempx*tam_poblacion+tempy;
            memcpy(&particion_Tablero[posIndividuosLocal[i]],&particion_Tablero[temporal],sizeof(struct Persona));
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            Reset_Casilla(particion_Tablero,temporal);
          }
        }
        else
        {
          MPI_Send(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
          MPI_Recv(&temp[0],tam_fila, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
          for(k=0;k<tam_fila;k++)
          {
            if(temp[k].valido == 0 && temp[k].posicion[1] == tempy)
            {
              temporal = posIndividuosLocal[i];
              posIndividuosLocal[i] = tempx*tam_poblacion+tempy;
              memcpy(&temp[k],&particion_Tablero[temporal],sizeof(struct Persona));
              temp[k].posicion[0]=tempx;
              temp[k].posicion[1]=tempy;
              Reset_Casilla(particion_Tablero,temporal);
	      break;
            }
          }
          MPI_Send(&temp[0],tam_fila, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
          MPI_Recv(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
        arriba=0;
        noposible=0;
      }
    }
  }
  else
  {
    for(i = 0; i<tam_poblacion;i++)
    {
      if(particion_Tablero[posIndividuosLocal[i]].estado != Fallecido && particion_Tablero[posIndividuosLocal[i]].valido == 1 )
      {
        x = particion_Tablero[posIndividuosLocal[i]].velocidad[0];
        y = particion_Tablero[posIndividuosLocal[i]].velocidad[1];
        particion_Tablero[posIndividuosLocal[i]].velocidad[0] = -3 + rand()% 6;
        particion_Tablero[posIndividuosLocal[i]].velocidad[1] = -3 + rand()% 6;
        id = particion_Tablero[posIndividuosLocal[i]].id;
        tempx=particion_Tablero[posIndividuosLocal[i]].posicion[0]+x;
        tempy=particion_Tablero[posIndividuosLocal[i]].posicion[1]+y;
        if(tempx >= (num_filas*(my_rank+1)-1)){tempx = num_filas;abajo=0;}
        else if (tempx < my_rank*num_filas){arriba=1;tempx = num_filas*my_rank-1;}
        if(tempy >= (num_filas-1))tempy=(num_filas-1);
        else if (tempy < 0)tempx = 0;
        if(arriba == 0 && arriba == 0)
        {
          for(j=0;j<tam_poblacion;j++)
          {
            if(particion_Tablero[posIndividuosLocal[j]].id == id);
            else if(particion_Tablero[posIndividuosLocal[j]].posicion[0] == tempx && particion_Tablero[posIndividuosLocal[j]].posicion[1] == tempy)
            {
              noposible = 1;
              break;
            }
          }
          if(noposible == 0 && (tempx!=particion_Tablero[posIndividuosLocal[i]].posicion[0] && tempy!=particion_Tablero[posIndividuosLocal[i]].posicion[0]))
          {
            temporal = posIndividuosLocal[i];
            posIndividuosLocal[i] = tempx*tam_poblacion+tempy;
            memcpy(&particion_Tablero[posIndividuosLocal[i]],&particion_Tablero[temporal],sizeof(struct Persona));
            particion_Tablero[posIndividuosLocal[i]].posicion[0]=tempx;
            particion_Tablero[posIndividuosLocal[i]].posicion[1]=tempy;
            Reset_Casilla(particion_Tablero,temporal);
          }
        }
        else
        {
          for(j = 0; j<tam_poblacion;j++)
          {
            auxX=posIndividuos[j]/tam_fila;
            auxY=posIndividuos[j]%tam_poblacion;
            if(auxX == tempx && auxY ==tempy)
            {
              noposible = 1;
              break;
            }
          }
          if(noposible == 0 && (tempx!=particion_Tablero[posIndividuosLocal[i]].posicion[0] && tempy!=particion_Tablero[posIndividuosLocal[i]].posicion[0]))
          {
            if(arriba==1)
            {
              MPI_Send(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
              MPI_Recv(&temp[0],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
              for(k=0;k<tam_fila;k++)
              {
                if(temp[k].valido == 0 && temp[k].posicion[1] == tempy)
                {
                  temporal = posIndividuosLocal[i];
                  posIndividuosLocal[i] = tempx*tam_poblacion+tempy;
                  memcpy(&temp[k],&particion_Tablero[temporal],sizeof(struct Persona));
                  temp[k].posicion[0]=tempx;
                  temp[k].posicion[1]=tempy;
                  Reset_Casilla(particion_Tablero,temporal);
		  break;
                }
              }
              MPI_Send(&temp[0],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
              MPI_Recv(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            }
            else
            {
              MPI_Send(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
              MPI_Recv(&temp[0],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
              for(k=0;k<tam_fila;k++)
              {
                if(temp[k].valido == 0 && temp[k].posicion[1] == tempy)
                {
                  temporal = posIndividuosLocal[i];
                  posIndividuosLocal[i] = tempx*tam_poblacion+tempy;
                  memcpy(&temp[k],&particion_Tablero[temporal],sizeof(struct Persona));
                  temp[k].posicion[0]=tempx;
                  temp[k].posicion[1]=tempy;
                  Reset_Casilla(particion_Tablero,temporal);
		  break;
                }
              }
              MPI_Send(&temp[0],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
              MPI_Recv(&particion_Tablero[x*tam_fila],tam_fila, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            }
          }
        }
        arriba=0;
        abajo=0;
        noposible=0;
      }
    }
  }
}
void cambiar_estado_poblacion(int *posIndividuos,int *posIndividuosLocal,struct Persona *particion_Tablero,struct Poblacion *Poblacion,int tam_vector,int * Vacunados_Muertos)
{

  int i,j,x,y,id,personas_en_particion;
  for(i=0;i<tam_vector;i++)
  {
    if(posIndividuosLocal[i]==0) break;
    else personas_en_particion++;
  }
  for(i = 0; i<personas_en_particion;i++) //Se miran los que estan en el procesador
  {
    if((particion_Tablero[posIndividuosLocal[i]].estado == Infectado || particion_Tablero[posIndividuosLocal[i]].estado == Sin_Sintomas) )
    {
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
     x = particion_Tablero[posIndividuosLocal[i]].posicion[0];
     y = particion_Tablero[posIndividuosLocal[i]].posicion[1];
     id = particion_Tablero[posIndividuosLocal[i]].id;
     for(j=0;j<personas_en_particion;j++)
     {
       if(particion_Tablero[posIndividuosLocal[j]].id == id);
       else if((particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]+3) ||(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]-3))
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
       else if((particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]+2) ||(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]-2))
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
       else if((particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]+1) ||(particion_Tablero[posIndividuosLocal[j]].posicion[0] == particion_Tablero[posIndividuosLocal[i]].posicion[0]-1))
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
       else if((particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]+3) ||(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]-3))
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
       else if((particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]+2) ||(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]-2))
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
       else if((particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]+1) ||(particion_Tablero[posIndividuosLocal[j]].posicion[1] == particion_Tablero[posIndividuosLocal[i]].posicion[1]-1))
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
}
//Funcion para cambio de estado mirando los procesadores vecinos
//La idea es que el procesador que ejecute envie a sus compañeros colindantes la posicion de una persona del borde
//Y estos le respondan si existe alguien o no y luego el calcule la probabilidad de contagiarse si esa persona esta contagiada
void cambiar_estado_poblacion_bordes(int dimension_particion,int tam_Poblacion,int *posIndividuos,int *posIndividuosLocal,struct Persona *particion_Tablero,struct Poblacion *Poblacion,int * Vacunados_Muertos, int tam_fila, int my_rank,int world_size, MPI_Datatype *PersonaType)
{
  int i,j,x,y,xaux,yaux;
  int numero_filas = dimension_particion/tam_fila;
  struct Persona *temp = malloc(sizeof(struct Persona));
  int posicion;
  if(my_rank==0)
  {
    for(i = 0; i<tam_Poblacion;i++)
    {
     if(posIndividuosLocal[i]==0);
     else
     {
       posicion= posIndividuosLocal[i]/numero_filas;
       if(posicion == numero_filas-1)
       {
         posicion = particion_Tablero[posIndividuosLocal[i]].id;
         x=posIndividuos[posicion]/tam_fila;
         y=posIndividuos[posicion]%tam_fila;
         for(j = 0; j <tam_Poblacion;j++)
         {
           xaux=posIndividuos[j]/tam_fila;
           yaux=posIndividuos[j]%tam_fila;
           if(xaux == x+1 && yaux == y)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[i]],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp,1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x +1 && yaux == y+1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x +1 && yaux == y-1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           if(temp->estado==Sin_Sintomas || temp->estado== Infectado)
           {
             if(contagiar_o_no(temp->p_muerte))
             {
               particion_Tablero[posIndividuosLocal[j]].estado = Sin_Sintomas;
               Vacunados_Muertos[2]++;
               break;
             }
            }
          }
        }
      }
    }
  }
  else if(my_rank == world_size-1)
  {
    for(i = 0; i<tam_Poblacion;i++)
    { 
     if(posIndividuosLocal[i]==0);
     else
     {
       posicion= posIndividuosLocal[i]/numero_filas;
       if(posicion == 0)
       {
         posicion = particion_Tablero[posIndividuosLocal[i]].id;
         x=posIndividuos[posicion]/tam_fila;
         y=posIndividuos[posicion]%tam_fila;
         for(j = 0; j <tam_Poblacion;j++)
         {
           xaux=posIndividuos[j]/tam_fila;
           yaux=posIndividuos[j]%tam_fila;
           if(xaux == x-1 && yaux == y)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[i]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp,1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x -1 && yaux == y+1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x -1 && yaux == y-1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           if(temp->estado==Sin_Sintomas || temp->estado== Infectado)
           {
             if(contagiar_o_no(temp->p_muerte))
             {
               particion_Tablero[posIndividuosLocal[j]].estado = Sin_Sintomas;
               Vacunados_Muertos[2]++;
               break;
             }
           }
          }
        }
      }
    }
  }
  else
  {
    for(i = 0; i<tam_Poblacion;i++)
    { 
     if(posIndividuosLocal[i]==0);
     else
     {
       posicion= posIndividuosLocal[i]/numero_filas;
       if(posicion == 0)
       {
         posicion = particion_Tablero[posIndividuosLocal[i]].id;
         x=posIndividuos[posicion]/tam_fila;
         y=posIndividuos[posicion]%tam_fila;
         for(j = 0; j <tam_Poblacion;j++)
         {
           xaux=posIndividuos[j]/tam_fila;
           yaux=posIndividuos[j]%tam_fila;
           if(xaux == x-1 && yaux == y)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[i]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp,1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x -1 && yaux == y+1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x -1 && yaux == y-1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           if(temp->estado==Sin_Sintomas || temp->estado== Infectado)
           {
             if(contagiar_o_no(temp->p_muerte))
             {
               particion_Tablero[posIndividuosLocal[j]].estado = Sin_Sintomas;
               Vacunados_Muertos[2]++;
               break;
             }
           }
          }
        }
        else if(posicion == numero_filas-1)
        {
         posicion = particion_Tablero[posIndividuosLocal[i]].id;
         x=posIndividuos[posicion]/tam_fila;
         y=posIndividuos[posicion]%tam_fila;
         for(j = 0; j <tam_Poblacion;j++)
         {
           xaux=posIndividuos[j]/tam_fila;
           yaux=posIndividuos[j]%tam_fila;
           if(xaux == x+1 && yaux == y)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[i]],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp,1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x +1 && yaux == y+1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           else if(xaux == x +1 && yaux == y-1)
           {
             MPI_Send(&particion_Tablero[posIndividuosLocal[j]],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD);
             MPI_Recv(&temp[0],1, *PersonaType, my_rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           }
           if(temp->estado==Sin_Sintomas || temp->estado== Infectado)
           {
             if(contagiar_o_no(temp->p_muerte))
             {
               particion_Tablero[posIndividuosLocal[j]].estado = Sin_Sintomas;
               Vacunados_Muertos[2]++;
               break;
             }
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
  j,vacunas= 0;
    if (vacunaDiaria>=0.999)
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
void Personas_en_mi(struct Persona *particion_Tablero,int* posIndividuos,int* posIndividuosLocal,int dimension_local)
{
  int i,j;
  j= 0;
  for(i=0;i<dimension_local;i++)
  {
    if(particion_Tablero[i].valido!=0)
    {
      posIndividuosLocal[j]=i;
      j++;
    }
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

  //MPI_Datatype PoblacionType;
  //Crear_Tipo_Poblacion(Poblacion->individuos, Poblacion->media_edad, Poblacion->radio,Poblacion->p_contagio,Poblacion->incubacion,Poblacion->recuperacion,Poblacion->cambio_vel,&PoblacionType);
  MPI_Barrier(MPI_COMM_WORLD);
  //MPI_Bcast(Poblacion, tamano_poblacion, PoblacionType, 0, MPI_COMM_WORLD);
 
  //Reparticion del tablero y vector de posiciones
  MPI_Bcast(posIndividuos, tam_Poblacion, MPI_INT, 0, MPI_COMM_WORLD);
 
  MPI_Barrier(MPI_COMM_WORLD);
  printf("Hola\n");
  //El scatter no esta bien hay que definir un tipo que va a enviar
  MPI_Scatter(Tablero,dimension_local,PersonaType,particion_Tablero,dimension_local,PersonaType,0,MPI_COMM_WORLD);//El scatter lo hace bien el problema es que al acceder a la informacion que se envia está mal
  printf("Hola 2\n");

  printf("Hola, soy el procesador %d y ya tengo mi trozo de la matriz \n",world_rank);
  int *posIndividuosLocal = (int*)malloc(tam_Poblacion*sizeof(int));
  Personas_en_mi(particion_Tablero,posIndividuos,posIndividuosLocal,dimension_local);
  int t,vacunas=0;
  int *Vacunados_Muertos = (int *)malloc(5* sizeof(int));
  for(i=0;i<5;i++) Vacunados_Muertos[i]=0;
  float tempVacuna=0.0;
  while( t < Simulacion ) //Bucle Principal
  {
    cambiar_estado_poblacion(posIndividuos,posIndividuosLocal,particion_Tablero,Poblacion,tam_Poblacion, Vacunados_Muertos);
    cambiar_estado_poblacion_bordes(dimension_local,tam_Poblacion,posIndividuos,posIndividuosLocal,particion_Tablero,Poblacion,Vacunados_Muertos, tam_tablero, world_rank,world_size, &PersonaType);
    mover_persona(particion_Tablero,posIndividuos, posIndividuosLocal,tam_tablero, tam_Poblacion, dimension_local, world_rank,world_size, &PersonaType);
    if(world_rank==0)
    {
     tempVacuna = vacunaDiaria+tempVacuna;
     while(tempVacuna >=1)
     {
      vacunas++;                        //El proceso de vacunacion debe realizarlo un unico procesador
      tempVacuna--;                     //Para que no todos vacunen el mismo numero de personas y se inmunice a la poblacion instantaneamente
     }
     vacunar_persona(Tablero, posIndividuos,vacunas,tam_Poblacion,Vacunados_Muertos);
     vacunas=0;
    }
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
   for(z=0;z<tam_Poblacion;z++)
   {
    fprintf(fb,"Soy yo %d y estoy en la posicion %d %d y estoy %d y mivelocidad es de %d %d\n",Tablero[posIndividuos[z]].id,Tablero[posIndividuos[z]].posicion[0],Tablero[posIndividuos[z]].posicion[1],Tablero[posIndividuos[z]].estado,Tablero[posIndividuos[z]].velocidad[0],Tablero[posIndividuos[z]].velocidad[1]);
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
  free(posIndividuos);
  free(Tablero);
  MPI_Finalize();
}