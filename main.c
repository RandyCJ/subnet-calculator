#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <math.h>
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

int check_big_mask(unsigned char *mask_array, struct Subnet_data *subnet_datas){
    unsigned int mask = mask_array[0] | mask_array[1] << 8 | mask_array[2] << 16 | mask_array[3] << 24;
    unsigned int tmp_mask = mask;

    int cidr = 0;
    while (tmp_mask != 0){
        tmp_mask = tmp_mask & (tmp_mask-1);
        cidr += 1;
    }

    if (cidr <= 0 || cidr > 32){
        return -4;
    }
    subnet_datas->cidr = cidr;
    struct Subnet_data tmp;
    short_mask_to_dotted_mask(&tmp, cidr);
    unsigned int correct_mask = tmp.mask[0] | tmp.mask[1] << 8 | tmp.mask[2] << 16 | tmp.mask[3] << 24;
    
    if (correct_mask == mask){
        return 0;
    }
    return -4;
}

int verify_ip_mask(char *ip_mask, struct Subnet_data *subnet_datas, int type){
    char tmp[100];
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
        subnet_datas->cidr = mask;
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

    if (type == 1 && flag == 0){
        flag = check_big_mask(subnet_datas->mask, subnet_datas);
    }

    return flag;
}

void get_ip_mask(struct Subnet_data *subnet_datas, char *user_string, char *directive_token){
    char tmp[1024];
    strcpy(tmp, user_string);
    char* token = strtok(tmp, directive_token);


    char ip_address[50];
    strcpy(ip_address, token);

    token = strtok(NULL, " MASK ");
    char mask[50];
    strcpy(mask, token);

    int flag = 0;
    flag = verify_ip_mask(ip_address, subnet_datas, 0);
    int tmp_flag = verify_ip_mask(mask, subnet_datas, 1);
    if (tmp_flag != 0) flag = tmp_flag;
    subnet_datas->operation = flag;
}

void print_ip_data(struct Subnet_data *subnet_datas){
    printf("IP ADDRESS: ");
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
    //strcat(regex, "$");

    broadcast_ip = regcomp( &reegex, regex, REG_EXTENDED);
    broadcast_ip = regexec( &reegex, user_string, 0, NULL, 0);

    if (broadcast_ip == 0){
        printf("\nBROADCAST\n");
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
        printf("\nNETWORK NUMBER\n");
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
    //strcat(regex3, "$");

    hosts_range = regcomp( &reegex, regex3, REG_EXTENDED);
    hosts_range = regexec( &reegex, user_string, 0, NULL, 0);

    if (hosts_range == 0){
        printf("\nHOSTS RANGE\n");
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

char *build_hosts_range(unsigned char *ip1, unsigned char *ip2){
    char network_number[40] = "";
    char number_str[5];

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", ip1[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
        else strcat(network_number, " - ");
    }

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", ip2[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
        else strcat(network_number, "\n");
    }

    char *tmp = network_number;
    return tmp;
}

void get_network_number(struct Subnet_data *subnet_datas){
    for (int i=0; i<4; i++){
        subnet_datas->network_address[i] = subnet_datas->ip[i] & subnet_datas->mask[i];
    }
}

void get_broadcast_ip(struct Subnet_data *subnet_datas){
    get_network_number(subnet_datas);
    for (int i=0; i<4; i++){
        subnet_datas->broadcast_ip[i] = subnet_datas->network_address[i] + (~subnet_datas->mask[i]);
    }
}

void get_hosts_range(struct Subnet_data *subnet_datas){
    get_network_number(subnet_datas);
    get_broadcast_ip(subnet_datas);
    for (int i=0; i<4; i++){
        subnet_datas->starting_address[i] = subnet_datas->network_address[i];
        subnet_datas->last_address[i] = subnet_datas->broadcast_ip[i];
    }
    subnet_datas->starting_address[3] += 1;
    subnet_datas->last_address[3] -= 1;
}

char *get_response(struct Subnet_data *subnet_datas){
    int operation = subnet_datas->operation;

    switch (operation){
    case 0:
        get_broadcast_ip(subnet_datas);
        return array_to_dotted_ip(subnet_datas->broadcast_ip);
    
    case 1:
        get_network_number(subnet_datas);
        return array_to_dotted_ip(subnet_datas->network_address);

    case 2:
        get_hosts_range(subnet_datas);
        if (subnet_datas->cidr > 30){
            return "There isn't a usable host range for cidr mask greater than 30\n";
        }
        return build_hosts_range(subnet_datas->starting_address, subnet_datas->last_address);
    
    case -1:
        return "You either entered a wrong directive or didn't entered a positive number with 1, 2 or 3 digits.\n";
    
    case -2:
        return "Numbers in ip address and mask must be greater than -1 and lower than 256.\n";

    case -3:
        return "CIDR masks must be greater than 0 and lower than 33.\n";
    
    case -4:
        return "You entered an invalid mask. Make sure to use CIDR value between 1 and 32\n";

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



// // //PRUEBAS
// int main(){
//     struct Subnet_data subnet_datas;
//     char *prueba;
//     char *response;
    
//     prueba = "GET BROADCAST IP 198.168.0.1 MASK 255.255.0.0";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Broadcast ip: %s; Esperado: 192.168.255.255\n", response);

//     prueba = "GET NETWORK NUMBER IP 198.168.0.1 MASK 255.255.0.0";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Network number: %s; Esperado: 192.168.0.0\n", response);

//     prueba = "GET HOSTS RANGE IP 198.168.0.1 MASK 255.255.0.0";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Hosts range: %s; Esperado: 	192.168.0.1 - 192.168.255.254\n", response);
    

//     prueba = "GET BROADCAST IP 172.16.0.56 MASK /25";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Broadcast ip: %s; Esperado: 172.16.0.127\n", response);

//     prueba = "GET NETWORK NUMBER IP 172.16.0.56 MASK /25";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Network number: %s; Esperado: 172.16.0.0\n", response);
    
//     prueba = "GET HOSTS RANGE IP 172.16.0.56 MASK /25";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Hosts range: %s; Esperado: 	172.16.0.1 - 172.16.0.126\n", response);


//     prueba = "GET BROADCAST IP 10.0.8.5 MASK /12";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Broadcast ip: %s; Esperado: 10.15.255.255\n", response);
    
//     prueba = "GET NETWORK NUMBER IP 10.0.8.5 MASK /12";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Network number: %s; Esperado: 10.0.0.0\n", response);

//     prueba = "GET HOSTS RANGE IP 10.0.8.5 MASK /12";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Hosts range: %s; Esperado: 	10.0.0.1 - 10.15.255.254\n", response);


//     prueba = "GET BROADCAST IP 10.8.2.5 MASK /29";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Broadcast ip: %s; Esperado: 10.8.2.7\n", response);
    
//     prueba = "GET NETWORK NUMBER IP 10.8.2.5 MASK /29";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Network number: %s; Esperado: 10.8.2.0\n", response);

//     prueba = "GET HOSTS RANGE IP 10.8.2.5 MASK /29";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Hosts range: %s; Esperado: 	10.8.2.1 - 10.8.2.6\n", response);


//     prueba = "GET BROADCAST IP 201.203.117.214 MASK /5";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Broadcast ip: %s; Esperado: 207.255.255.255\n", response);
    
//     prueba = "GET NETWORK NUMBER IP 201.203.117.214 MASK /5";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Network number: %s; Esperado: 200.0.0.0\n", response);

//     prueba = "GET HOSTS RANGE IP 201.203.117.214 MASK /5";
//     verify_operation(prueba, &subnet_datas);
//     print_ip_data(&subnet_datas);
//     response = get_response(&subnet_datas);
//     printf("Hosts range: %s; Esperado: 200.0.0.1 - 207.255.255.254\n", response);
    
//     return 0;
// }

