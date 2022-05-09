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

int verify_ip_mask(char *ip_mask, struct Subnet_data *subnet_datas, int type){
    char tmp[20];
    strcpy(tmp, ip_mask);
    regex_t reegex;
    int is_short_mask = regcomp( &reegex, "^/", REG_EXTENDED);
    is_short_mask = regexec( &reegex, tmp, 0, NULL, 0);
    int flag = 0;

    if (is_short_mask == 0){
        int mask = atoi(strtok(tmp, "/"));
        if (mask < 8 || mask > 32){
            flag = -1;
        }
        //tmp mientras veo como cambiar mask corta a larga
        subnet_datas->mask[0] = 255;
        subnet_datas->mask[1] = 255;
        subnet_datas->mask[2] = 255;
        subnet_datas->mask[3] = 0;
        return flag;
    }

    int i = 0;
    char *token = strtok(tmp, ".");
    while (token != NULL) {
        unsigned int tmp = atoi(token);
        if (type == 0) subnet_datas->ip[i] = tmp;
        else subnet_datas->mask[i] = tmp;
        if (tmp > 255){
            flag = -1;
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
    if (verify_ip_mask(ip_address, subnet_datas, 0) < 0){
        flag = -2;
    }
    if (verify_ip_mask(mask, subnet_datas, 1) < 0){
        flag = -3;
    }
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
    strcat(regex2, "$");

    network_number = regcomp( &reegex, regex2, REG_EXTENDED);
    network_number = regexec( &reegex, user_string, 0, NULL, 0);

    if (network_number == 0){
        get_ip_mask(subnet_datas, user_string, NETWORK_NUMBER);
        //subnet_datas = get_ip_mask(user_string, NETWORK_NUMBER);
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
        get_ip_mask(subnet_datas, user_string, HOSTS_RANGE);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 2;
        }
        return;
    }
}

int main(){
    struct Subnet_data subnet_datas;
    char *prueba = "GET BROADCAST IP 198.168.0.1 MASK 255.212.5.128";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);

    prueba = "GET NETWORK NUMBER IP 112.12.12.6 MASK 255.5.25.128";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);


    prueba = "GET HOSTS RANGE IP 214.126.9.6 MASK 255.5.25.328";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);

    prueba = "GET BROADCAST IP 14.332.4.243 MASK /8";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);
    
    prueba = "GET NETWORK NUMBER IP 112.12.12.6 MASK /16";
    verify_operation(prueba, &subnet_datas);
    print_ip_data(&subnet_datas);


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

