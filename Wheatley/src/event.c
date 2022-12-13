#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "event.h"



static org_t* org=NULL; //Variable interna de la "libreria"
volatile static int ON_state_cnt;
volatile static int OFF_state_cnt;
//static org_t* organizador=NULL; //Variable externa
org_t* init_org(org_t **_org, int _capacidad){   //You have to pass the pointer to the pointer... bruh
//ok, recibimos el puntero al puntero de la posicion del struct
    *_org=(org_t*)malloc(sizeof(org_t)); //Damos una seccion de espacio para _org. Aqui estamos escribiendo en el puntero el puntero a la posicion alocada por malloc
    (*_org)->ON_procs=(proc_t*)malloc(_capacidad*sizeof(proc_t));
    (*_org)->OFF_procs=(proc_t*)malloc(_capacidad*sizeof(proc_t));
    assert((*_org)->ON_procs!=NULL && (*_org)->OFF_procs != NULL);
    
    //(*_org).n_elements=0;
    org=*_org;
    return org;
}
static int proc_t_cmp(const void * a, const void * b){
    //if(((proc_t*)a)->timestamp>=((proc_t*)b)->timestamp)return 1;
    //return -1;
    return ((((proc_t*)a)->timestamp > ((proc_t*)b)->timestamp) - (((proc_t*)a)->timestamp < ((proc_t*)b)->timestamp));
}
int set_event(proc_t *proc, int *element_counter, int _ID, int tim){
    //Refactoring
    int i=0;
    do{
        if(((proc+i)->ID == _ID) | (i>=org->n_ON_elements)){
            (proc+i)->ID = _ID;
            (proc+i)->timestamp = tim;
            break;
        }
        i++;
    }while(i<=*element_counter);
    if(i>=*element_counter) element_counter++; //THIS FUCKER (Together made it apear as it was working)
    qsort(proc, *element_counter, sizeof(proc_t), proc_t_cmp);
}
proc_t *del_event(proc_t *proc, int *element_counter, int _ID){
    //Refactoring
    int i = *element_counter;
    while(i != 0){
        i--;
        if((proc+i)->ID == _ID){
            element_counter--;
            if(i != (*element_counter)){
                memcpy(proc+i, element_counter, sizeof(proc_t));
            }
            break;
        }
    }
    qsort(proc, element_counter, sizeof(proc_t), proc_t_cmp);
    return proc;
}
org_t* set_ON_event(int _ID, int on_tim){
    int i=0;
    do{
        if(((org->ON_procs+i)->ID == _ID) | (i>=org->n_ON_elements)){
            (org->ON_procs+i)->ID = _ID;
            (org->ON_procs+i)->timestamp = on_tim;
            break;
        }
        i++;
    }while(i<=org->n_ON_elements);
    if(i>=(org->n_ON_elements)) org->n_ON_elements++; //THIS FUCKER (Together made it apear as it was working)
    qsort(org->ON_procs, org->n_ON_elements, sizeof(proc_t), proc_t_cmp);
    return org;
}
org_t* set_OFF_event(int _ID, int off_tim){
    int i=0;
    do{
        if(((org->OFF_procs+i)->ID == _ID) | (i>=org->n_OFF_elements)){
            (org->OFF_procs+i)->ID = _ID;
            (org->OFF_procs+i)->timestamp = off_tim;
            break;
        }
        i++;
    }while(i<=org->n_OFF_elements);
    if(i>=(org->n_OFF_elements)) org->n_OFF_elements++; //THIS FUCKER 
    qsort(org->OFF_procs, org->n_OFF_elements, sizeof(proc_t), proc_t_cmp);
    return org;
}
org_t* set_full_event(int _ID, int on_tim, int off_tim){
    set_event(org->ON_procs, &(org->n_ON_elements), _ID, on_tim);
    set_event(org->OFF_procs, &(org->n_OFF_elements), _ID, off_tim);
    //set_ON_event(_ID, on_tim);
    //set_OFF_event(_ID, off_tim);
    return org;
}
org_t* del_full_event(int _ID){
    //del_event(org->ON_procs, org->n_ON_elements, _ID);
    //del_event(org->OFF_procs, org->n_OFF_elements, _ID);
    int i = org->n_ON_elements;
    while(i != 0){
        i--;
        if((org->ON_procs+i)->ID == _ID){
            if(i==(org->n_ON_elements-1)){
                org->n_ON_elements--;
                break;
            }
            else{
                org->n_ON_elements--;
                memcpy(org->ON_procs+i, org->ON_procs+org->n_ON_elements, sizeof(proc_t));
                break;
            }
        }
    }
    i = org->n_OFF_elements;
    while(i != 0){
        i--;
        if((org->OFF_procs+i)->ID == _ID){
            if(i==(org->n_OFF_elements-1)){
                org->n_OFF_elements--;
                break;
            }
            else{
                org->n_OFF_elements--;
                memcpy(org->OFF_procs+i, org->OFF_procs+org->n_OFF_elements, sizeof(proc_t));
                break;
            }
        }
    }
    qsort(org->ON_procs, org->n_ON_elements, sizeof(proc_t), proc_t_cmp);
    qsort(org->OFF_procs, org->n_OFF_elements, sizeof(proc_t), proc_t_cmp);
    
    return org;
}
int del_all_events(){
    org->n_ON_elements = 0;
    org->n_OFF_elements = 0;
    (org->ON_procs)->ID = 0;    //Si no se pone a 0, ret_actual_ON_event_ID() devuelve un ID
    (org->OFF_procs)->ID = 0;   //Que ya no existe
    return 0;
}
int ret_next_ON_event_time(){
    if((ON_state_cnt+1)>=(org->n_ON_elements)){
        ON_state_cnt=0;
    }else{
        ON_state_cnt=ON_state_cnt+1;
    }
    return (org->ON_procs+ON_state_cnt)->timestamp;
}
int ret_next_OFF_event_time(){
    if((OFF_state_cnt+1)>=(org->n_OFF_elements)){
        OFF_state_cnt=0;
    }else{
        OFF_state_cnt=OFF_state_cnt+1;
    }
    return (org->OFF_procs+OFF_state_cnt)->timestamp;
}
int ret_actual_ON_event_ID(){
    //return (org->n_ON_elements != 0) ? (org->ON_procs+ON_state_cnt)->ID : 0;
    //Hacer la operacion previamente mejora tiempo de respuesta por 0.01 ms
    return (org->ON_procs+ON_state_cnt)->ID;
}
int ret_actual_OFF_event_ID(){
    //return (org->n_OFF_elements != 0) ? (org->OFF_procs+OFF_state_cnt)->ID : 0;
    //Hacer la operacion previamente mejora tiempo de respuesta por 0.01 ms
    return (org->OFF_procs+OFF_state_cnt)->ID;
}
//int set_Secuential_event(int _ID, byte t){

//}
