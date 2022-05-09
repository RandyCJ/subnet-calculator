#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include "main.h"

#define BROADCAST_IP "GET BROADCAST IP "
#define NETWORK_NUMBER "GET NETWORK NUMBER IP "
#define HOSTS_RANGE "GET HOSTS RANGE IP "
#define RANDOM_SUBNETS "GET RANDOM SUBNETS NETWORK NUMBER "
#define REGEX_IP_MASK "[0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9] \
MASK ([0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]|/[0-9]?[0-9])"

void short_mask_to_dotted_mask(struct Subnet_data *subnet_datas, int mask){
    unsigned int shift = 32 - mask;
    unsigned char full_mask[4] = {1, 1, 1, 1};

    if (shift < 8){
        subnet_datas->mask[3] = ((full_mask[3] << shift)-1) ^ 255;
        subnet_datas->mask[2] = 0 ^ 255;
        subnet_datas->mask[1] = 0 ^ 255;
        subnet_datas->mask[0] = 0 ^ 255;
    }
    else if (shift < 16){
        subnet_datas->mask[3] = 255 ^ 255;
        subnet_datas->mask[2] = (full_mask[2] << shift-8)-1 ^ 255;
        subnet_datas->mask[1] = 0 ^ 255;
        subnet_datas->mask[0] = 0 ^ 255;
    }
    else if (shift < 24){
        subnet_datas->mask[3] = 255 ^ 255;
        subnet_datas->mask[2] = 255 ^ 255;
        subnet_datas->mask[1] = (full_mask[1] << shift-16)-1 ^ 255;
        subnet_datas->mask[0] = 0 ^ 255;
    }
    else{
        subnet_datas->mask[3] = 255 ^ 255;
        subnet_datas->mask[2] = 255 ^ 255;
        subnet_datas->mask[1] = 255 ^ 255;
        subnet_datas->mask[0] = (full_mask[0] << shift-24)-1 ^ 255;
    }
}

int verify_ip_mask(char *ip_mask, struct Subnet_data *subnet_datas, int type){
    char tmp[20];
    strcpy(tmp, ip_mask);
    regex_t reegex;
    int is_short_mask = regcomp( &reegex, "^/", REG_EXTENDED);
    is_short_mask = regexec( &reegex, tmp, 0, NULL, 0);
    int flag = 0;

    if (is_short_mask == 0){
        int mask = atoi(strtok(tmp, "/"));
        if (mask < 1 || mask > 32){
            return -3;
        }
        short_mask_to_dotted_mask(subnet_datas, mask);
        return flag;
    }

    int i = 0;
    char *token = strtok(tmp, ".");
    while (token != NULL) {
        int tmp = atoi(token);
        if (type == 0) subnet_datas->ip[i] = tmp;
        else subnet_datas->mask[i] = tmp;
        if (tmp > 255){
            flag = -2;
        }
        i++;
        token = strtok(NULL, ".");
    }
    return flag;
}

void get_ip_mask(struct Subnet_data *subnet_datas, char *user_string, char *directive_token){
    char tmp[16];
    strcpy(tmp, user_string);
    char* token = strtok(tmp, directive_token);

    char ip_address[32];
    strcpy(ip_address, token);

    token = strtok(NULL, " MASK ");
    char mask[32];
    strcpy(mask, token);

    int flag = 0;
    flag = verify_ip_mask(ip_address, subnet_datas, 0);
    int tmp_flag = verify_ip_mask(mask, subnet_datas, 1);
    if (tmp_flag != 0) flag = tmp_flag;
    subnet_datas->operation = flag;
}

void print_ip_data(struct Subnet_data *subnet_datas){
    printf("\nIP ADDRESS: ");
    for (int i=0; i<4; i++){
        printf("%d", subnet_datas->ip[i]);
        if (i != 3){
            printf(".");
        }
    }
    printf("\nMASK: ");
    for (int i=0; i<4; i++){
        printf("%d", subnet_datas->mask[i]);
        if (i != 3){
            printf(".");
        }
    }
    printf("\nOPERATION: %d\n", subnet_datas->operation);
}

void verify_operation(char *user_string, struct Subnet_data *subnet_datas){
    regex_t reegex;
    subnet_datas->operation = -1;

    //Verifying if BROADCAST IP
    int broadcast_ip;
    char regex[200] = "^";
    strcat(regex, BROADCAST_IP);
    strcat(regex, REGEX_IP_MASK);
    strcat(regex, "$");

    broadcast_ip = regcomp( &reegex, regex, REG_EXTENDED);
    broadcast_ip = regexec( &reegex, user_string, 0, NULL, 0);

    if (broadcast_ip == 0){
        printf("BROADCAST\n");
        get_ip_mask(subnet_datas, user_string, BROADCAST_IP);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 0;
        }
        return;
    }

    //Verifying if NETWORK NUMBER
    int network_number;
    char regex2[200] = "^";
    strcat(regex2, NETWORK_NUMBER);
    strcat(regex2, REGEX_IP_MASK);
    //strcat(regex2, "$");

    network_number = regcomp( &reegex, regex2, REG_EXTENDED);
    network_number = regexec( &reegex, user_string, 0, NULL, 0);

    if (network_number == 0){
        printf("NETWORK NUMBER\n");
        get_ip_mask(subnet_datas, user_string, NETWORK_NUMBER);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 1;
        }
        return;
    }

    //Verifying if HOSTS RANGE
    int hosts_range;
    char regex3[200] = "^";
    strcat(regex3, HOSTS_RANGE);
    strcat(regex3, REGEX_IP_MASK);
    strcat(regex3, "$");

    hosts_range = regcomp( &reegex, regex3, REG_EXTENDED);
    hosts_range = regexec( &reegex, user_string, 0, NULL, 0);

    if (hosts_range == 0){
        printf("HOSTS RANGE\n");
        get_ip_mask(subnet_datas, user_string, HOSTS_RANGE);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 2;
        }
        return;
    }
}

char *array_to_dotted_ip(unsigned char *ip){
    char network_number[20] = "";
    char number_str[5];

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", ip[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
        else strcat(network_number, "\n");
    }
    char *tmp = network_number;
    return tmp;
}

char *get_network_number(struct Subnet_data *subnet_datas){
    unsigned char network_number[4];
    
    for (int i=0; i<4; i++){
        network_number[i] = subnet_datas->ip[i] & subnet_datas->mask[i];
    }

    return array_to_dotted_ip(network_number);
}

char *get_response(struct Subnet_data *subnet_datas){
    int operation = subnet_datas->operation;
    printf("Operacion a realizar: %d\n", operation);

    switch (operation){
    // case 0:
    //     return get_broadcast_ip(subnet_datas);
    
    case 1:
        return get_network_number(subnet_datas);
    
    // case 2:
    //     return get_hosts_range(subnet_datas);
    
    case -1:
        return "You either entered a wrong directive or didn't entered a positive number with 1, 2 or 3 digits.\n";
    
    case -2:
        return "Numbers in ip address and mask must be greater than -1 and lower than 256.\n";

    case -3:
        return "Short masks must be greater than 0 and lower than 33.\n";

    default:
        return "Something went wrong.\n";
    }
}

char *subnet_calculator(char *user_input){
    struct Subnet_data subnet_datas;
    verify_operation(user_input, &subnet_datas);
    print_ip_data(&subnet_datas);
    return get_response(&subnet_datas);
}

/*
int main(){
    struct Subnet_data subnet_datas;
    char *prueba;
    char *response;
    /*
    prueba = "GET BROADCAST IP 198.168.0.1 MASK 255.212.5.128";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);
    


    prueba = "GET NETWORK NUMBER IP 172.16.0.56 MASK 255.255.255.128";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);
    response = get_response(&subnet_datas);
    printf("Network number: %s\n", response);

    //printf("Network number ip: %s\n", response);

    // char network_ip[20] = "";
    // printf("Network IP: ");
    // for (int i=0; i<4; i++){
    //     strcat(network_ip, response[i]);
    //     if (i != 3) strcat(network_ip, ".");
    // }
    
    // printf("%s\n", network_ip);


    /*
    prueba = "GET HOSTS RANGE IP 214.126.9.6 MASK 255.5.25.328";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);

    prueba = "GET BROADCAST IP 14.332.4.243 MASK /8";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);
    
    prueba = "GET NETWORK NUMBER IP 112.12.12.6 MASK /16";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);

    /*
    prueba = "GET HOSTS RANGE IP 214.126.9.6 MASK /32";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);


    prueba = "GET BROADCAST IP 1723.16.0.56 MASK 255.5.25.128";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);

    prueba = "GET NETWORK NUMBER IP 133.16.0.56 MASK /7";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);

    prueba = "GET HOSTS RANGE IP 173.16.0.56 MASK /33";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);
    
    return 0;
}

*/