# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "codexion.h"

t_dongle init(
    unsigned int dongle_id,
    long released_time,
    t_parameter s_parameter,
    pthread_mutex_t mutex,
    pthread_cond_t condition
){
    t_dongle dongle;
    
    dongle.dongle_id = dongle_id;
    dongle.released_time = released_time;
    dongle.s_parameter = s_parameter;
    dongle.mutex = mutex;
    dongle.condition = condition;

    return dongle;
}

t_dongle *init_dongles(t_parameter params, int N)
{
    t_dongle *dongles = malloc(sizeof(t_dongle) * N);
    for (int i = 0; i < N; i++)
    {
        dongles[i].dongle_id = i;
        dongles[i].released_time = 0;
        dongles[i].s_parameter = params;
        pthread_mutex_init(&dongles[i].mutex, NULL);
        pthread_cond_init(&dongles[i].condition, NULL);
    }
    return (dongles)
}

t_coder init_coders(t_parameter params, int N)
{
    t_coder *coders = malloc(sizeof(t_dongle) * N);
    for (int i = 0; i < N; i++)
    {
        coders[i].coder_id = i;
        coders[i].left_hand = NULL;
        coders[i].right_hand = NULL;
        coders[i].parameter =params;
    }
}




t_parameter init_param(t_parameter params)
{
    int N = params.number_of_coders;
    t_dongle *dongles = *init_dongles(params, N);
    t_coder *coders = *init_coders(params, N);
    
}