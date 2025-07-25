#include "Parameter_CPU.h"

extern struct uwsc_client *cl;

void reset_settings_lamp_group(void)
{
    lamp_gr1_port1.rear = -1;
    lamp_gr1_port1.front = -1;

    lamp_gr1_port2.rear = -1;
    lamp_gr1_port2.front = -1;
    
    lamp_gr2_port1.rear = -1;
    lamp_gr2_port2.front = -1;

    lamp_gr2_port2.rear = -1;
    lamp_gr2_port2.front = -1;

    lamp_gr3_port1.rear = -1;
    lamp_gr3_port1.front = -1;

    lamp_gr3_port2.rear = -1;
    lamp_gr3_port2.front = -1;

    // Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port1_file, ADDR_STORAGE_GROUP1_PORT1, lamp_gr1_port1);
    // Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port2_file, ADDR_STORAGE_GROUP1_PORT2, lamp_gr1_port2);
    // Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port1_file, ADDR_STORAGE_GROUP2_PORT1, lamp_gr2_port1);
    // Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port2_file, ADDR_STORAGE_GROUP2_PORT2, lamp_gr2_port2);
    // Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port1_file, ADDR_STORAGE_GROUP3_PORT1, lamp_gr3_port1);
    // Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port2_file, ADDR_STORAGE_GROUP3_PORT2, lamp_gr3_port2);
}
void get_all_settings_storage_lamp_group(void)
{   
    lamp_gr1_port1 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr1_port1_file, ADDR_STORAGE_GROUP1_PORT1, lamp_gr1_port1);
    lamp_gr1_port2 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr1_port2_file, ADDR_STORAGE_GROUP1_PORT2, lamp_gr1_port2);
    lamp_gr2_port1 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr2_port1_file, ADDR_STORAGE_GROUP2_PORT1, lamp_gr2_port1);
    lamp_gr2_port2 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr2_port2_file, ADDR_STORAGE_GROUP2_PORT2, lamp_gr2_port2);
    lamp_gr3_port1 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr3_port1_file, ADDR_STORAGE_GROUP3_PORT1, lamp_gr3_port1);
    lamp_gr3_port2 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr3_port2_file, ADDR_STORAGE_GROUP3_PORT2, lamp_gr3_port2);

    sort_down_lamp_group(lamp_gr1_port1.inp_lamp_arr, lamp_gr1_port1.rear);
    sort_down_lamp_group(lamp_gr1_port2.inp_lamp_arr, lamp_gr1_port2.rear);
    sort_down_lamp_group(lamp_gr2_port1.inp_lamp_arr, lamp_gr2_port1.rear);
    sort_down_lamp_group(lamp_gr2_port2.inp_lamp_arr, lamp_gr2_port2.rear);
    sort_down_lamp_group(lamp_gr3_port1.inp_lamp_arr, lamp_gr3_port1.rear);
    sort_down_lamp_group(lamp_gr3_port2.inp_lamp_arr, lamp_gr3_port2.rear);
}

void Write_struct_time_active_toFile(FILE *file, char *link, time_active_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(time_active_t), 1, file);
    fclose(file);
}

time_active_t Read_struct_time_active_toFile(FILE *file, char *link, time_active_t data_Write){
    file = fopen(link, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while (fread(&data_Write, sizeof(time_active_t), 1, file));
    printf("Start: hh:mm -> %d:%d\r\n End: hh:mm -> %d:%d\r\n", data_Write.hh_start1,
           data_Write.mm_start1,
           data_Write.hh_end1,
           data_Write.mm_end1);
    fclose(file);
    return data_Write;
}

void Write_struct_dim_active_toFile(FILE *file, char *link, dim_active_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(dim_active_t), 1, file);
    fclose(file);
}

dim_active_t Read_struct_dim_active_toFile(FILE *file, char *link, dim_active_t data_Write){
    file = fopen(link, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while (fread(&data_Write, sizeof(time_active_t), 1, file));
    printf("Dim: x -> %d\r\n", data_Write.dim);
    fclose(file);
    return data_Write;
}

void Write_struct_dim_schedule_active_toFile(FILE *file, char *link, dim_active_schedule_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(dim_active_schedule_t), 1, file);
    fclose(file);
}

dim_active_schedule_t Read_struct_dim_schedule_active_toFile(FILE *file, char *link, dim_active_schedule_t data_Write){
    file = fopen(link, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while (fread(&data_Write, sizeof(dim_active_schedule_t), 1, file));
    fclose(file);
    return data_Write;
}

void Write_struct_counter_lamp_toFile(FILE *file, char *link, type_configPackage_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(type_configPackage_t), 1, file);
    fclose(file);
}

type_configPackage_t Read_struct_counter_lamp_toFile(FILE *file, char *link, type_configPackage_t data_Write){
    file = fopen(link, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while (fread(&data_Write, sizeof(type_configPackage_t), 1, file));
    printf("Counter: x -> %d\r\n", counters_package.val1);
    fclose(file);
    return data_Write;
}

void Write_struct_sensor_value_toFile(FILE *file, char *link, type_mtlc_sensor_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(type_mtlc_sensor_t), 1, file);
    fclose(file);
}

type_mtlc_sensor_t Read_struct_sensor_value_toFile(FILE *file, char *link, type_mtlc_sensor_t data_Write){
    file = fopen(link, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while (fread(&data_Write, sizeof(type_mtlc_sensor_t), 1, file));
    fclose(file);
    return data_Write;
}

current_time_t getCurrent_Time(time_t rawtime, struct tm *timeinfo){
    rawtime = time(0);
    timeinfo = localtime(&rawtime);
    // printf ( "Current local time and date: %s", asctime (timeinfo) );
    current_time.hh_current = timeinfo->tm_hour;
    current_time.mm_current = timeinfo->tm_min;
    current_time.ss_current = timeinfo->tm_sec;
    // printf("hh:mm current is: %d:%d\n", current_time.hh_current, current_time.mm_current);
    return current_time;
}

void Write_struct_time_on_off_toFile(FILE *file, char *link, time_on_off_t data_Write){
	
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(time_on_off_t), 1, file);
    fclose(file);
}

time_on_off_t Read_struct_time_on_off_toFile(FILE *file, char *link, time_on_off_t data_Write){
    file = fopen(link, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while (fread(&data_Write, sizeof(time_on_off_t), 1, file));
	// for (int i = 0; i <  data_Write.size_array_on_off_schedule; i++)
	// {
		// printf("\n%d.Start: hh:mm -> %d:%d\r\n End: hh:mm -> %d:%d\r\n", 
		   // i,
		   // data_Write.hh_start[i],
           // data_Write.mm_start[i],
           // data_Write.hh_end[i],
           // data_Write.mm_end[i]);
	// }
	// printf("\nsizearray: %d\r\n", data_Write.size_array_on_off_schedule);
    fclose(file);
    return data_Write;
}

void Write_struct_setting_toFile(FILE *file, char *link, setting_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(setting_t), 1, file);
    fclose(file);
}

setting_t Read_struct_setting_toFile(FILE *file, char *link, setting_t data_Write){
    file = fopen(link, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while (fread(&data_Write, sizeof(setting_t), 1, file));
    printf("Server: %d\r\n", setting.server);
    fclose(file);
    return data_Write;
}


void Write_struct_queue_toFile(FILE *file, char *link, xqueue_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(xqueue_t), 1, file);
    fclose(file);
}

xqueue_t Read_struct_queue_toFile(FILE *file, char *link, xqueue_t data_Write){
    file = fopen(link, "r");
    if(file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while(fread(&data_Write, sizeof(xqueue_t), 1, file));
    printf("xqueue data rear & front: %d, %d\r\n", queue.rear, fread(&data_Write, sizeof(xqueue_t), 1, file));
    fclose(file);
    return data_Write;
}

void Write_struct_queue_lamps_port1_toFile(FILE *file, char *link, xqueue_lamp_port1_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(xqueue_lamp_port1_t), 1, file);
    fclose(file);
}

xqueue_lamp_port1_t Read_struct_queue_lamps_port1_toFile(FILE *file, char *link, xqueue_lamp_port1_t data_Write){
    file = fopen(link, "r");
    if(file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while(fread(&data_Write, sizeof(xqueue_lamp_port1_t), 1, file));
    fclose(file);
    return data_Write;
}

void Write_struct_queue_lamps_port2_toFile(FILE *file, char *link, xqueue_lamp_port2_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(xqueue_lamp_port2_t), 1, file);
    fclose(file);
}

xqueue_lamp_port2_t Read_struct_queue_lamps_port2_toFile(FILE *file, char *link, xqueue_lamp_port2_t data_Write){
    file = fopen(link, "r");
    if(file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while(fread(&data_Write, sizeof(xqueue_lamp_port2_t), 1, file));
    fclose(file);
    return data_Write;
}

void Write_struct_queue_lamps_group_toFile(FILE *file, char *link, xqueue_lamp_group_t data_Write){
    file = fopen(link, "w");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opened file\n");
        exit(1);
    }
    fwrite(&data_Write, sizeof(xqueue_lamp_group_t), 1, file);
    fclose(file);
}

xqueue_lamp_group_t Read_struct_queue_lamps_group_toFile(FILE *file, char *link, xqueue_lamp_group_t data_Write){
    file = fopen(link, "r");
    if(file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    while(fread(&data_Write, sizeof(xqueue_lamp_group_t), 1, file));
    fclose(file);
    return data_Write;
}

void sort_up_lamp_group(uint8_t number[], int n)
{
    int temp;
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (number[i] > number[j])
            {
                temp = number[i];
                number[i] = number[j];
                number[j] = temp;
            }
        }
    }
}

void sort_down_lamp_group(uint8_t a[], int n)
{
    for (int i = 0; i <= n - 1; i++)
        for (int j = i; j <= n; j++)
        {
            if (a[i] < a[j])
            {
                uint8_t temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
}

void insert_queue_lamp_group(uint8_t insert_item, xqueue_lamp_group_t *lamp_group, int port)
{
    bool enable_save_queue = true;
    int i;
    if(port == 1)
    {
        if (lamp_group->rear == Size_Queue_Array - 1)
        {
            printf("Overflow\r\n");
        }
        else
        {
            if (lamp_group->front == -1)
            {
                lamp_group->front = 0;
            }
            for (i = 0; i <= lamp_group->rear; i++)
            {
                if (insert_item == lamp_group->inp_lamp_arr[i])
                {
                    enable_save_queue = false;
                    break;
                }
            }
            if (enable_save_queue)
            {
                printf("Element to be inserted lamp group rs485 in the Queue:\r\n");
                lamp_group->rear = lamp_group->rear + 1;
                printf("gia tri QUEUE REAR: %d\r\n",lamp_group->rear);
                lamp_group->inp_lamp_arr[lamp_group->rear] = insert_item;
            }
            else
            {
                printf("Exist element in the Queue!!!!!\r\n");
                printf("gia tri QUEUE REAR: %d\r\n",lamp_group->rear);
            }
        }
    }
    else if (port == 2)
    {
        if (lamp_group->rear == Size_Queue_Array - 1)
        {
            printf("Overflow\r\n");
        }
        else
        {
            if (lamp_group->front == -1)
            {
                lamp_group->front = 0;
            }
            for (i = 0; i <= lamp_group->rear; i++)
            {
                if (insert_item == lamp_group->inp_lamp_arr[i])
                {
                    enable_save_queue = false;
                    break;
                }
            }
            if (enable_save_queue)
            {
                printf("Element to be inserted lamp port2 rs485 in the Queue:\r\n");
                lamp_group->rear = lamp_group->rear + 1;
                lamp_group->inp_lamp_arr[lamp_group->rear] = insert_item;
            }
            else
            {
                printf("Exist element in the Queue!!!!!\r\n");
                printf("gia tri QUEUE REAR: %d\r\n",lamp_group->rear);
            }
        }
    }
}

void descrease_queue_lamp_group(xqueue_lamp_group_t *lamp_group, int port)
{
    if(port == 1)
    {
        if (lamp_group->front == -1 || lamp_group->front > lamp_group->rear)
        {
            printf("Underflow \n");
            lamp_group->front = -1;
            lamp_group->rear = -1;
        }
        else
        {
            printf("Element deleted from the Queue group port 1: %d\n", lamp_group->inp_lamp_arr[lamp_group->front]);
            lamp_group->front = lamp_group->front + 1;
            printf("gia tri lamp_group front group port 1: %d\r\n", lamp_group->front);
            if (lamp_group->front > lamp_group->rear)
            {

                lamp_group->front = -1;
                lamp_group->rear = -1;
            }
        }
        
    }
    else if (port == 2)
    {
        if (lamp_group->front == -1 || lamp_group->front > lamp_group->rear)
        {
            printf("Underflow \n");
            lamp_group->front = -1;
            lamp_group->rear = -1;
        }
        else
        {
            printf("Element deleted from the Queue lamp port 2: %d\n", lamp_group->inp_lamp_arr[lamp_group->front]);
            lamp_group->front = lamp_group->front + 1;
            printf("gia tri queue front lamp port 2: %d\r\n", lamp_group->front);
            if (lamp_group->front > lamp_group->rear)
            {
                lamp_group->front = -1;
                lamp_group->rear = -1;
            }
        }
    }
}

void show_queue_lamp_group(xqueue_lamp_group_t *lamp_group, int port)
{
    if(port == 1)
    {
        if (lamp_group->front == -1)
        {
            printf("Empty lamp group port 1\n");
        }
        else
        {
            printf("monitor lamp_group \n");
            for (int i = lamp_group->front; i <= lamp_group->rear; i++)
                printf("%d ", lamp_group->inp_lamp_arr[i]); //
            printf("\n");
        }
    }
    else if(port == 2)
    {
        if (lamp_group->front == -1)
        {
            printf("Empty lamp group port 2 \n");
        }
        else
        {
            printf("monitor lamp group port 2: \n");
            for (int i = lamp_group->front; i <= lamp_group->rear; i++)
                printf("%d ", lamp_group->inp_lamp_arr[i]); 
            printf("\n");
        }
    }
}

void send_lamp_group_cpu_to_qt(xqueue_lamp_group_t *lamp_gr1, xqueue_lamp_group_t *lamp_gr2, const char* template, const char cmd)
{                
    char data_tmp[1000] = {0};
    char tmp_mess_port1_1[200] = {0};
    char tmp_mess_port1_2[200] = {0};
    char tmp_mess_port2_1[200] = {0};
    char tmp_mess_port2_2[200] = {0};
    
    // sort_up(arr, rear);
    sort_up_lamp_group(lamp_gr1->inp_lamp_arr, lamp_gr1->rear);
    for(int i = 0 ; i <= lamp_gr1->rear; i ++){
        if(lamp_gr1->inp_lamp_arr[i] != 0)
        {
            snprintf(tmp_mess_port1_2, sizeof(tmp_mess_port1_2) + 2, "%s %d", tmp_mess_port1_1, lamp_gr1->inp_lamp_arr[i]);
            snprintf(tmp_mess_port1_1, sizeof(tmp_mess_port1_1), "%s", tmp_mess_port1_2);
        }
    }
    sort_up_lamp_group(lamp_gr2->inp_lamp_arr, lamp_gr2->rear);
    for(int i = 0 ; i <= lamp_gr2->rear; i ++){
        if(lamp_gr2->inp_lamp_arr[i] != 0)
        {
            snprintf(tmp_mess_port2_2, sizeof(tmp_mess_port2_2) + 2, "%s %d", tmp_mess_port2_1, lamp_gr2->inp_lamp_arr[i]);
            snprintf(tmp_mess_port2_1, sizeof(tmp_mess_port2_1), "%s", tmp_mess_port2_2);
        }
    }
    printf("feedback message: %s\r\n", tmp_mess_port1_1);
    printf("feedback message: %s\r\n", tmp_mess_port2_1);
    int data_len3 = sprintf(data_tmp, template, cmd, tmp_mess_port1_1, tmp_mess_port2_1);

    cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len3, data_tmp);
    printf("Da send den qt  group ok %s\n", data_tmp);
    memset(data_tmp, 0x00, 1000);
}