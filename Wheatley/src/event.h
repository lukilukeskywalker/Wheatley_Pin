#ifndef EVENT_H    
#define EVENT_H   
//#include "event.c"
typedef struct {
    int ID;
    int timestamp;
}proc_t;
typedef struct {
    int n_ON_elements;
    int n_OFF_elements;
    proc_t *ON_procs;   //Puntero a posicion de procesos a encender
    proc_t *OFF_procs; //Puntero a posicion de procesos a apagar
}org_t;
org_t* init_org(org_t **, int);
org_t* set_ON_event(int _ID, int on_tim);
org_t* set_OFF_event(int _ID, int off_tim);
org_t* set_full_event(int _ID, int on_tim, int off_tim);
org_t* del_full_event(int _ID);
int del_all_events();
int ret_next_ON_event_time();
int ret_next_OFF_event_time();
int ret_actual_ON_event_ID();
int ret_actual_OFF_event_ID();
#endif 